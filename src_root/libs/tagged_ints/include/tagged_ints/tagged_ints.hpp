/*
 * tagged_ints.hpp
 *
 *  Created on: Nov 10, 2019
 *      Author: vsol
 */

#ifndef SRC_ROOT_LIBS_TAGGED_INTS_INCLUDE_TAGGED_INTS_TAGGED_INTS_HPP_
#define SRC_ROOT_LIBS_TAGGED_INTS_INCLUDE_TAGGED_INTS_TAGGED_INTS_HPP_

namespace pet_ll
{
namespace details
{
	template<class T> concept Integral = std::is_integral<T>::value;
}

template<class >
class tagged_int
{

};


}



#endif /* SRC_ROOT_LIBS_TAGGED_INTS_INCLUDE_TAGGED_INTS_TAGGED_INTS_HPP_ */
