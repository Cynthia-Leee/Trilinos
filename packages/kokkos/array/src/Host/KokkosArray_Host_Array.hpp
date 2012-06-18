/*
//@HEADER
// ************************************************************************
// 
//          Kokkos: Node API and Parallel Node Kernels
//              Copyright (2008) Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER
*/

#ifndef KOKKOS_HOST_ARRAY_HPP
#define KOKKOS_HOST_ARRAY_HPP

#include <Host/KokkosArray_Host_IndexMap.hpp>

#include <KokkosArray_Host_macros.hpp>
#include <impl/KokkosArray_Array_macros.hpp>
#include <KokkosArray_Clear_macros.hpp>


namespace KokkosArray {
namespace Impl {

//----------------------------------------------------------------------------

template< typename ArrayType >
struct Factory< Array< ArrayType , Host > , void >
{
  typedef Array< ArrayType , Host >  output_type ;

  static output_type create( const std::string & label , size_t nP )
  {
    typedef MemoryManager< Host > memory_manager ;
    typedef typename output_type::value_type value_type ;

    output_type array ;

    array.m_index_map.template assign< value_type >(nP);
    array.m_data.allocate( array.m_index_map.allocation_size() , label );

    HostParallelFill<value_type>( array.m_data.ptr_on_device() , 0 ,
                                  array.m_index_map.allocation_size() );

    return array ;
  }
};

//----------------------------------------------------------------------------

template< typename ArrayType , class Device >
struct Factory< Array< ArrayType , HostMapped< Device > > , void >
{
  typedef Array< ArrayType , HostMapped< Device > >  output_type ;

  static output_type create( const std::string & label , size_t nP )
  {
    typedef MemoryManager< Host > memory_manager ;
    typedef typename output_type::value_type value_type ;

    output_type array ;

    array.m_index_map.template assign< value_type >(nP);
    array.m_data.allocate( array.m_index_map.allocation_size() , label );

    HostParallelFill<value_type>( array.m_data.ptr_on_device() , 0 ,
                                  array.m_index_map.allocation_size() );

    return array ;
  }
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

template< typename ArrayType >
struct Factory< Array< ArrayType , Host > , Array< ArrayType , Host > >
{
public:
  typedef Array< ArrayType , Host > output_type ;
  typedef Array< ArrayType , Host > input_type ;

  static inline
  void deep_copy( const output_type & output ,
                  const input_type  & input )
  {
    typedef typename output_type::value_type value_type ;

    HostParallelCopy<value_type,value_type>( output.ptr_on_device() ,
                                             input. ptr_on_device() ,
                                             output.m_index_map.allocation_size() );
  }

  // Called by create_mirror
  static inline
  output_type create( const input_type & input )
  {
    return Factory< output_type , void >
             ::create( std::string(), input.dimension(0) );
  }
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

template< typename ArrayType , class Device >
struct Factory< Array< ArrayType , Host > ,
                Array< ArrayType , HostMapped< Device > > >
{
  typedef Array< ArrayType , Host >                 output_type ;
  typedef Array< ArrayType , HostMapped< Device > > input_type ;

  inline static
  void deep_copy( const output_type & output , const input_type & input )
  {
    typedef typename output_type::value_type value_type ;

    HostIndexMapDeepCopy< value_type,
                          typename output_type::index_map ,
                          typename input_type ::index_map >
      ::deep_copy( output.m_data , output.m_index_map ,
                   input .m_data , input .m_index_map );
  }
};

template< typename ArrayType , class Device >
struct Factory< Array< ArrayType , HostMapped< Device > > ,
                Array< ArrayType , Host > >
{
  typedef Array< ArrayType , HostMapped< Device > > output_type ;
  typedef Array< ArrayType , Host >                 input_type ;

  inline static
  void deep_copy( const output_type & output , const input_type & input )
  {
    typedef typename output_type::value_type value_type ;

    HostIndexMapDeepCopy< value_type,
                          typename output_type::index_map ,
                          typename input_type ::index_map >
      ::deep_copy( output.m_data , output.m_index_map ,
                   input .m_data , input .m_index_map );
  }
};

//----------------------------------------------------------------------------

} // namespace Impl
} // namespace KokkosArray

#endif /* #ifndef KOKKOS_HOST_ARRAY_HPP */

