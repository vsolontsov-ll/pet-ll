# Thoughts
## Logic (design)
/dev/shm buffer to build.

## Writers (encoders)
Basically the data can be written with/without respect of alignment.
Full CLs can be flushed with CLFLUSHOPT or written back with CLWB 
(which probably doesn't make much sense but let's see).
Finally, writer can add padding to the CL, so that recording is done with CL granularity.

So the traits are:
1. Regular vs NT stores
1. Packed / unpacked -- **It impacts the methods required per type (size, alignment) and calculation of record size**
1. Padded to CL or not
1. Persistent strategy
    1. none
    1. CLFLUSHOPT
    1. CLFLUSH
1. Serialization strategy
    1. none
    1. SFENCE

## Buffers management

Possible buffers are
1. Direct mem buffer (anonymous/private, tmpfs, fsdax)
1. Direct file buffers (there could be tricks )
1. Async -- Another thread takes care of dumping buffers
    1. Passive -- Logger uses whatever buffer it has. No space in buffer means no log.
    1. Active -- there's a queue of buffers. Once a buffer complete, logger enqueues it for dumping

    
## TODOs
1. BasicWriter (packed, cached, no SFENCE)
    1. Optimize size check
    1. Reader and Writer to codec (write and read are strongly related)
1. Basic buffer
1. Rework to custom section