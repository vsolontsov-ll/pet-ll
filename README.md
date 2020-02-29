# Low latency logger
## Overview
Logger is a traditional pain for C++ developers. There're many options and many of them are pretty good.
(See https://github.com/PlatformLab/NanoLog -- apart from NanoLog it contains a comparison with competitors.)

However general purpose logger should be portable and feature-rich. Even with C++ where one of the key principles is
"you don't pay for what you don't use" there's still no "zero-cost abstraction" (thanks to Chandler Carruth for 
his fantastic talk on CPPCON 2019). 

Low latency "world" is a a land of trade-offs. Portability is usually one of potentially secrified feature. Here I'm 
going to ignore portability when it's profitable. Which means that to make it working on a different platform one would 
need to apply specifal efforts and may fail.

**Primary platform is x86_64/Linux**

Let's take a look at  usual time-consuming actions for dumping data:
1. make a decision whether to log or not (check log mask/log level)
    1. fetch log mask to a register (as it's in 99.999% of cases is dynamic, people don't want to recompile program just to change the log level!)
    2. branching 
2. Take a timestamp (clock_gettime() is vDSO which is fast, but still a function call)
3. Format a string to dump (huge waste of time!)
    1. Cache pollution -- to format a string it needs to convert data into a string representation 
    (directly in a log buffer or in a static buffer and copy it then)
4. Dump to a file/memory or enqueue for later dumping
File IO usually leads to syscalls -- expensive!
Dumping directly to memory mapped area is risky as kernel time to time needs to dump dirty pages. 
So at least (minimal optimistic need) to account new dirty state it needs to handle next write to the page.
Worst case a basic MOV instruction will be interrupted and stall while Kernel is writing-back the changes.

5. [optional] Synchronization
Async loggers may (and usually do) require synchronization accessing a queue.

## Key decisions
1. No data formatting -- dump binary, take formatting offline.
2. No heavy synchronization -- Logger object is not thread safe (one thread can use it at a time).

   However there could be thread-safe interface for maintenence (replacing log buffer and so on)

3. Timestamping mechanizm is configurable at compile-time. Default is RDTSC.
4. Data is segregated to static and dynamic. All about placement of logging record line source file:line, format 
string -- everthing we can we should take off-line.
5. [TODO] Later on it's possible to use self-modifying code and remove unnecessary branching for disabled/enabled log 
points. Static data could contain address of branching instruction, logging code start and end. So branching instruction 
could be replaced with JMP to (or NOP)

### Supported data types
* Built-in (int, const char*, ...) -- first cut plan
* String literals -- next iteration
* Complex objects. As an idea it can provide a format string, decoding format and serialization function at compile
time. 

### Why using .init_array
_TODO_ Revisit this part. Actually, .init_array is a hidden section used by glibc (and not only). But we easily can use
an own custom section either with pointers or something else. With a custom section we can get a deterministic 
order: constructor of a static object (mabe even thread-local logger) can walk through the section and collect all the 
information. Each binary (shared object) has to initialize its static objects (so has an entries in .init_array). And 
such an object with internal linkage can just copy data from custom section to a global array.

It's not easy writing low latency code. 
Permanently checking assembly generated by a compiler is annoying (but thanks to Mett Godbolt, he made it easier).
Sometimes it requires to add compiler-specific attributes or platform-specific calls/instructions.

So portability of this code is limited by definition. Based on requirements, partability/standard-compliece 
could be not a good reason for poor performance. 
In other words, squeezing a few CPU cycles could be considered as a good enough reason for using a trick which in 
all other scenarios must be rejected as a "dirty hack".   

Initarray is a section in ELF containing a flat array of pointers to functions. The functions to be called at startup.
C++ compilers usually uses this sections for initializing global and namespace-scoped static objects.

Different platforms may have different order of initialization of even ignore this section. So this approach **is not 
portable**.


## Classification of required information

1. Severity/record type (static) -- type of the record or (usually) its severity (DEBUG, INFO, WARNING, ...)

1. Timestamp (runtime) -- one of the most important attributes of the log record. Different timestamping mechanisms give 
different granularity, accuracy and correlation with machine "wall-clock". One may want to use system clock, 
steady clock, RDTSC or something else. Timestamping mechanism must be configurable at compile time.

1. Threads ID (startup) -- by design logger instance is single-threaded. So no good reason to repeat Thread ID 
again and again.

1. Location in source (static) -- \__FILE\__, \__LINE\__ and (a bit less) \__PRETTY\_FUNCTION\__ in log macros is 
a very regular practice. Whether you need it in output or not, once it's collected at compile time and doesn't add any 
runtime overhead, worth to have it.

1. Serialization format (static) -- this is a binary logger. It doesn't format records at runtime. How to dump 
the data into buffers must be known at compile time.

1. Parising format (static or startup) -- this format tells how to parse the data from binary buffers. Compiler knows 
the data types of fields to be dumped. However, without making assumptions about maximum number of fields, it's hard to
put this information and may require some non-constexpr activity.

1. Format string (static) -- in majority of cases you at compile time know how you would want to 
format your text in the log record. It's looks like a fair limitation for low latency. Also assuming that log record is
uniquely identified by FILE and LINE, offline converter may use different format. 

1. Dynamic data (runtime) -- the data your program actually aquired at run time. 

## Buffer management options
1. Unlimited -- A simplified usecase when at compile (or startup) time we're 100% sure thread can't run out of space
    * First cut it could be unsafe (and thus very easy to implement). 
    * Second step is also easy -- just allocate a guard page after the buffer without write access.
    * Full implementation on top of guard page needs to handle SIGSEGV, take actions and resume execution.
2. Limited -- Check it has enough space for every record. 
    * Happily majority of the data is fixed size, so record size in many cases is knonw at compile time.
    The rest (strings) would require a call to strlen(). Not a big deal
3. Replacable (limited).

   Another thread may advertise a new buffer. Logger is free to take it when needed.  E.g. bachground thread dumps RAM buffers to a file by completion (maintains a stack of buffers). Logger pulls a buffer out of the stack, evetually fills it and takes the next. Background thread takes care about dumping the buffers and returning them to the stack.
    
4. Mandatory replacable (limited) -- unlikely it's valuable...

   Another thread may request a logger to use a new buffer.
   E.g. another thread does msync on a memory-mapped file and up-front requires logger to use another buffer.


### No space processing options
1. Drop the record
    1. Silently
    2. Somehow register dropped records number
1. Allocate (request) and continue
1. Stall -- unlikely it's an option.

### Encoding options
1. Packed -- don't care about alignment, just pack the data sequentially byte after byte.
1. Aligned -- the data is packed according to alignment of 4 or 8 bytes (performance to be measured). Could be very 
good for rear and short records.
1. CL-aligned records -- start every record from a cache-line boundary, poison or zero the tail. This algorithm could 
be combined with either of the above. _Looks attractive for combining with non-temporal stores._

#### Storing and persisting
1. Cached stores
    1. CLFLUSH[OPT]
    1. CLWB
    1. CLDEMOTE -- *not available yet*
    1. [very unsafe and experiments] NT-store the same CL 
1. NT-stores
1. Either cached or NT stores followed by SFENCE -- this is to ensure sequence of reaching ADR domain. Makes sense only with buffers in NV RAM and may have very high delayed cost (at LOCK-ed operations or MFENCE).

## Processing stages
1. Dump startup info
1. Log -- produce the binary sequence for further processing
1. Persist -- make sure the binary sequence is persisted
    1. Sync -- e.g. there's NVRAM and we do take care about pushing the data down to ADR domain
    1. Async -- logger works with regular RAM, so there's a dedicated phase to save the data to a persistent media
1. Post-process
    1. Generate human-readable text using format strings and all the additional info.
    1. Combine to a chronologically sorted sequence. -- Multiple threads produce their records concurrently and there's no explicit order imposed by the logger. 
1. Filtering -- grep is fine.
1. Postmortem analyses -- in case of application crash some log info may still reside in shared memory or NVRAM ring buffers. Extraction of information is possible because 

## Attractive variants
### Fixed guarded buffer.
One disadventage of this schema about DTLB misses and even page-walks. As hot thread needs streaming to a huge buffer,
it will be filling page by page and at crossing page boundary may hit a DTLB or even STLB misses.

The buffer could be backed by
1. PMM (DAX file system or device).

   NV RAM is expensive, but it's probably the best available option for low latency processes.
   Intel Optane is slow and the latency maybe hit at locked operations but if logging thread has no locked operation, it should be fine.
   
   Another issue is that to really guarantee the persistency order SFENCES are required (as mentioned, they have delayed
   cost on LOCKed operations).
   
   Warning: to guarantee the persistency (not only relative order of it but the fact) MFENCE -- bloody expensive!

1. RAM

   Great option for development (if you have enough RAM) especially if you're targetting NV RAM for production.
   Also if you don't cansider OS or hardware crash as a risk, it's also fine for production.
   
1. Regular FS

   It won't give data persistency without msync or fsync which are too slow to be considered. 
   It's ok for development. But there're two drawbacks that make this option not very attractive for prod:
    1. Reliability in case of OS/HW crash
    1. Pagefaults duging or after writeback.
   
### Ring buffer or set of page-aligned buffers
The buffer can be backed by RAM or PMM (see all the persistency notes above). 
It assumes there's a background thread or process dealing with the binary data.

#### Thread vs process
In a simple case it looks attractive mimickingking a regular async logger: run a thread which processes the log data
from per-thread buffers. Formats the data to human-readable form, storts it in chronological order and writes to a file.

The drawbacks of a thread are:
1. Potentially unnecessary work at run-time.
1. A need in dedicated thread in every process
1. Potential inter-processor interrupts at modifying process address space (mapping/unmapping new pages). -- not really too harmful 

Drawbacks of process are:
1. Complexity. There should be a communication channel between the processes to tell dumping process there's a new logger or if a logger has gone.
1. Orchestration. Without background dumper buffers may get full very quickly.

### Persisting locally vs sending via network
Persisting locally is a must-have feature because of simplicity.
However sending logs to a centralized storage or log processing system is also a common requirement. But in any case 
segregation for static runtime information needs to be taken into account.

## Scalable and reliable variant
Summarizing the above we can see that temporary buffers in PMM combined with a dedicated process dumping log records
with or without conversion (or sending them out via network) looks like the best option.

The dumper process features can be choosen independently from the logging applications. It can use different techniques
for saving data:
   * regular open()/write() syscalls
   * Linux AIO
   * io_uring -- a new cool feature from Kernel 5.1

as well as sending data out:
   * BSD sockets
   * Firm-common transport library (based on vendors kernell-bypass libraries)
   * ...

## Components
### Log points registry
Shared library with a basic API to work with log points
* batch add log points (at startup or library load)
* [late phase] mark as removed to support reusing point ID at re-loading libraries
* Find point by ID
* Enumerate log point
* Notify about changes in the registry

### Logging macros
Set of macros responsible for 
1. registering log points with their attributes
2. conditionally (according to log mask) call the logger, passing log point ID and arguments

### Buffer
Basic chunk of memory (could be page-aligned).

### Super-buffer
Could represent a ring-buffer (wrap around) or set of buffers.

### Codec 
Code responsible for composing binary data in a buffer and parse the data from it.
All the variands around packed/aligned/CL-aligned to be reflected here.

### Writer
Takes care about stores (cached vs NT) and serialization (SFENCE). 

### Logger
Highly (statically) configurable object which composes Writer, Codec and Suport-buffer(s) together.

### Parser
Parses log records (uses registry). Provides an API for:
1. Obtaining a record size (e.g. to send the record via network or persist individual record).
2. Extract fields one by one (maybe via visitor pattern).

### Formatter
Format the record in human readable form.
Current assumption that libfmt will be used. Libfmt uses something like array of variants under the hood which can be 
generated at run-time. But there's no "public" interface for that -- only internal `fmt::internal::make_arg<>()`. But there's even a test `TEST(FormatTest, Dynamic)`.

<<TBD>>