/*
// @HEADER
// ***********************************************************************
//
//          Tpetra: Templated Linear Algebra Services Package
//                 Copyright (2008) Sandia Corporation
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
// ************************************************************************
// @HEADER
*/

#include "Tpetra_TestingUtilities.hpp"
#include "Tpetra_Map.hpp"
#include <type_traits>

namespace { // (anonymous)

  using Teuchos::outArg;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::REDUCE_MIN;
  using Teuchos::reduceAll;
  using std::endl;

  int numTimesToCreateNode = 1;

  TEUCHOS_STATIC_SETUP()
  {
    Teuchos::CommandLineProcessor& clp = Teuchos::UnitTestRepository::getCLP ();
    clp.setOption ("numTimesToCreateNode", &numTimesToCreateNode,
                   "Number of times to create (and destroy) a Node instance");
  }

  // Issue 510 involves creating a Node instance indirectly through
  // Map with a default 'node' argument.
  template<class LO, class GO, class NT>
  void
  testCreatingNodeWithMap (bool& success,
                           Teuchos::FancyOStream& out,
                           const Teuchos::RCP<const Teuchos::Comm<int> >& comm)
  {
    typedef Tpetra::Map<LO, GO, NT> map_type;

    const size_t lclNumInds = 5;
    const Tpetra::global_size_t gblNumInds = comm->getSize () * lclNumInds;
    const GO indexBase = 0;

    RCP<map_type> map0;
    TEST_NOTHROW( map0 = rcp (new map_type (gblNumInds, lclNumInds, indexBase, comm)) );
    TEST_ASSERT( ! map0.is_null () );
    if (! map0.is_null ()) {
      // Do something with the Map, to avoid premature compiler
      // optimization.  This only does something on Process 0, since
      // it uses the low verbosity level.  Thus, we don't need a
      // global check for whether the Map is null.
      map0->describe (out, Teuchos::VERB_LOW);
    }
    // At this point, Map's destructor gets called.  Depending on how
    // Map creates its Node, this may mean that the Node's destructor
    // also gets called.  This is relevant to #510.

    const int lclSuccess = success ? 1 : 0;
    int gblSuccess = 0; // to be set by the all-reduce below
    reduceAll<int, int> (*comm, REDUCE_MIN, lclSuccess, outArg (gblSuccess));
    TEST_EQUALITY_CONST( gblSuccess, 1 );
  }

  // The Teuchos unit test framework implicitly defines two input
  // arguments to this test:
  //
  // bool& success
  // Teuchos::FancyOStream& out
  TEUCHOS_UNIT_TEST( Node, Issue510 )
  {
    typedef typename Tpetra::Map<>::local_ordinal_type LO;
    typedef typename Tpetra::Map<>::global_ordinal_type GO;
    typedef typename Tpetra::Map<>::node_type NT;

    auto comm = Tpetra::TestingUtilities::getDefaultComm ();
    out << "Issue #510 test (create Node implicitly via Map's ctor)" << endl;
    Teuchos::OSTab tab0 (out);
    out << "Create Node " << numTimesToCreateNode
        << "time" << (numTimesToCreateNode != 1 ? "s" : "") << endl;

    for (int k = 0; k < numTimesToCreateNode; ++k) {
      // This function does an all-reduce on every call to check
      // success on all processes.  Thus, we don't need an all-reduce
      // at the end of the unit test.
      testCreatingNodeWithMap<LO, GO, NT> (success, out, comm);
    }
    out << "All done!" << endl;
  }

} // namespace (anonymous)
