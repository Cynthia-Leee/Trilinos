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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef TPETRA_IMPORT_UTIL2_HPP
#define TPETRA_IMPORT_UTIL2_HPP

///
/// \file Tpetra_Import_Util2.hpp
/// \brief Utility functions for packing and unpacking sparse matrix entries.
///

#include "Tpetra_ConfigDefs.hpp"
#include "Tpetra_Import.hpp"
#include "Tpetra_HashTable.hpp"
#include "Tpetra_Map.hpp"
#include "Tpetra_Util.hpp"
#include "Tpetra_Distributor.hpp"
#include "Tpetra_Details_reallocDualViewIfNeeded.hpp"
#include "Kokkos_DualView.hpp"
#include <Teuchos_Array.hpp>
#include <utility>

// Tpetra::CrsMatrix uses the functions below in its implementation.
// To avoid a circular include issue, only include the declarations
// for CrsMatrix.  We will include the definition after the functions
// here have been defined.
#include "Tpetra_CrsMatrix_decl.hpp"

namespace Tpetra {
namespace Import_Util {

/// \brief Sort the entries of the (raw CSR) matrix by column index
///   within each row.
template<typename Scalar, typename Ordinal>
void
sortCrsEntries (const Teuchos::ArrayView<size_t>& CRS_rowptr,
                const Teuchos::ArrayView<Ordinal>& CRS_colind,
                const Teuchos::ArrayView<Scalar>&CRS_vals);


template<typename rowptr_array_type, typename colind_array_type, typename vals_array_type>
void
sortCrsEntries (const rowptr_array_type& CRS_rowptr,
                const colind_array_type& CRS_colind,
                const vals_array_type& CRS_vals);

/// \brief Sort and merge the entries of the (raw CSR) matrix by
///   column index within each row.
///
/// Entries with the same column index get merged additively.
template<typename Scalar, typename Ordinal>
void
sortAndMergeCrsEntries (const Teuchos::ArrayView<size_t>& CRS_rowptr,
                        const Teuchos::ArrayView<Ordinal>& CRS_colind,
                        const Teuchos::ArrayView<Scalar>& CRS_vals);

/// \brief lowCommunicationMakeColMapAndReindex
///
/// If you know the owning PIDs already, you can make the colmap a lot
/// less expensively.  If LocalOrdinal and GlobalOrdinal are the same,
/// you can (and should) use the same array for both columnIndices_LID
/// and columnIndices_GID.  This routine works just fine "in place."
///
/// Note: The owningPids vector (on input) should contain owning PIDs
/// for each entry in the matrix, like that generated by
/// Tpetra::Import_Util::unpackAndCombineIntoCrsArrays routine.  Note:
/// This method will return a Teuchos::Array of the remotePIDs, used for
/// construction of the importer.
///
/// \warning This method is intended for expert developer use only,
///   and should never be called by user code.
template <typename LocalOrdinal, typename GlobalOrdinal, typename Node>
void
lowCommunicationMakeColMapAndReindex (const Teuchos::ArrayView<const size_t> &rowPointers,
                                      const Teuchos::ArrayView<LocalOrdinal> &columnIndices_LID,
                                      const Teuchos::ArrayView<GlobalOrdinal> &columnIndices_GID,
                                      const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > & domainMap,
                                      const Teuchos::ArrayView<const int> &owningPids,
                                      Teuchos::Array<int> &remotePids,
                                      Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > & colMap);


} // namespace Import_Util
} // namespace Tpetra


//
// Implementations
//

namespace Tpetra {
namespace Import_Util {

// Note: This should get merged with the other Tpetra sort routines eventually.
template<typename Scalar, typename Ordinal>
void
sortCrsEntries (const Teuchos::ArrayView<size_t> &CRS_rowptr,
                const Teuchos::ArrayView<Ordinal> & CRS_colind,
                const Teuchos::ArrayView<Scalar> &CRS_vals)
{
  // For each row, sort column entries from smallest to largest.
  // Use shell sort. Stable sort so it is fast if indices are already sorted.
  // Code copied from  Epetra_CrsMatrix::SortEntries()
  size_t NumRows = CRS_rowptr.size()-1;
  size_t nnz = CRS_colind.size();

  for(size_t i = 0; i < NumRows; i++){
    size_t start=CRS_rowptr[i];
    if(start >= nnz) continue;

    Scalar* locValues   = &CRS_vals[start];
    size_t NumEntries   = CRS_rowptr[i+1] - start;
    Ordinal* locIndices = &CRS_colind[start];

    Ordinal n = NumEntries;
    Ordinal m = 1;
    while (m<n) m = m*3+1;
    m /= 3;

    while(m > 0) {
      Ordinal max = n - m;
      for(Ordinal j = 0; j < max; j++) {
        for(Ordinal k = j; k >= 0; k-=m) {
          if(locIndices[k+m] >= locIndices[k])
            break;
          Scalar dtemp = locValues[k+m];
          locValues[k+m] = locValues[k];
          locValues[k] = dtemp;
          Ordinal itemp = locIndices[k+m];
          locIndices[k+m] = locIndices[k];
          locIndices[k] = itemp;
        }
      }
      m = m/3;
    }
  }
}

namespace Impl {

template<class RowOffsetsType, class ColumnIndicesType, class ValuesType>
class SortCrsEntries {
private:
  typedef typename ColumnIndicesType::non_const_value_type ordinal_type;
  typedef typename ValuesType::non_const_value_type scalar_type;

public:
  SortCrsEntries (const RowOffsetsType& ptr,
                  const ColumnIndicesType& ind,
                  const ValuesType& val) :
    ptr_ (ptr),
    ind_ (ind),
    val_ (val)
  {
    static_assert (std::is_signed<ordinal_type>::value, "The type of each "
                   "column index -- that is, the type of each entry of ind "
                   "-- must be signed in order for this functor to work.");
  }

  KOKKOS_FUNCTION void operator() (const size_t i) const
  {
    const size_t nnz = ind_.dimension_0 ();
    const size_t start = ptr_(i);

    if (start < nnz) {
      const size_t NumEntries = ptr_(i+1) - start;

      const ordinal_type n = static_cast<ordinal_type> (NumEntries);
      ordinal_type m = 1;
      while (m<n) m = m*3+1;
      m /= 3;

      while (m > 0) {
        ordinal_type max = n - m;
        for (ordinal_type j = 0; j < max; j++) {
          for (ordinal_type k = j; k >= 0; k -= m) {
            const size_t sk = start+k;
            if (ind_(sk+m) >= ind_(sk)) {
              break;
            }
            const scalar_type dtemp = val_(sk+m);
            val_(sk+m)   = val_(sk);
            val_(sk)     = dtemp;
            const ordinal_type itemp = ind_(sk+m);
            ind_(sk+m) = ind_(sk);
            ind_(sk)   = itemp;
          }
        }
        m = m/3;
      }
    }
  }

  static void
  sortCrsEntries (const RowOffsetsType& ptr,
                  const ColumnIndicesType& ind,
                  const ValuesType& val)
  {
    // For each row, sort column entries from smallest to largest.
    // Use shell sort. Stable sort so it is fast if indices are already sorted.
    // Code copied from  Epetra_CrsMatrix::SortEntries()
    // NOTE: This should not be taken as a particularly efficient way to sort
    // rows of matrices in parallel.  But it is correct, so that's something.
    if (ptr.dimension_0 () == 0) {
      return; // no rows, so nothing to sort
    }
    const size_t NumRows = ptr.dimension_0 () - 1;

    typedef size_t index_type; // what this function was using; not my choice
    typedef typename ValuesType::execution_space execution_space;
    // Specify RangePolicy explicitly, in order to use correct execution
    // space.  See #1345.
    typedef Kokkos::RangePolicy<execution_space, index_type> range_type;
    Kokkos::parallel_for ("sortCrsEntries", range_type (0, NumRows),
      SortCrsEntries (ptr, ind, val));
  }

private:
  RowOffsetsType ptr_;
  ColumnIndicesType ind_;
  ValuesType val_;
};

} // namespace Impl

template<typename rowptr_array_type, typename colind_array_type, typename vals_array_type>
void
sortCrsEntries (const rowptr_array_type& CRS_rowptr,
                const colind_array_type& CRS_colind,
                const vals_array_type& CRS_vals)
{
  Impl::SortCrsEntries<rowptr_array_type, colind_array_type,
    vals_array_type>::sortCrsEntries (CRS_rowptr, CRS_colind, CRS_vals);
}

// Note: This should get merged with the other Tpetra sort routines eventually.
template<typename Scalar, typename Ordinal>
void
sortAndMergeCrsEntries (const Teuchos::ArrayView<size_t> &CRS_rowptr,
                        const Teuchos::ArrayView<Ordinal> & CRS_colind,
                        const Teuchos::ArrayView<Scalar> &CRS_vals)
{
  // For each row, sort column entries from smallest to largest,
  // merging column ids that are identify by adding values.  Use shell
  // sort. Stable sort so it is fast if indices are already sorted.
  // Code copied from Epetra_CrsMatrix::SortEntries()

  if (CRS_rowptr.size () == 0) {
    return; // no rows, so nothing to sort
  }
  const size_t NumRows = CRS_rowptr.size () - 1;
  const size_t nnz = CRS_colind.size ();
  size_t new_curr = CRS_rowptr[0];
  size_t old_curr = CRS_rowptr[0];

  for(size_t i = 0; i < NumRows; i++){
    const size_t old_rowptr_i=CRS_rowptr[i];
    CRS_rowptr[i] = old_curr;
    if(old_rowptr_i >= nnz) continue;

    Scalar* locValues   = &CRS_vals[old_rowptr_i];
    size_t NumEntries   = CRS_rowptr[i+1] - old_rowptr_i;
    Ordinal* locIndices = &CRS_colind[old_rowptr_i];

    // Sort phase
    Ordinal n = NumEntries;
    Ordinal m = n/2;

    while(m > 0) {
      Ordinal max = n - m;
      for(Ordinal j = 0; j < max; j++) {
        for(Ordinal k = j; k >= 0; k-=m) {
          if(locIndices[k+m] >= locIndices[k])
            break;
          Scalar dtemp = locValues[k+m];
          locValues[k+m] = locValues[k];
          locValues[k] = dtemp;
          Ordinal itemp = locIndices[k+m];
          locIndices[k+m] = locIndices[k];
          locIndices[k] = itemp;
        }
      }
      m = m/2;
    }

    // Merge & shrink
    for(size_t j=old_rowptr_i; j < CRS_rowptr[i+1]; j++) {
      if(j > old_rowptr_i && CRS_colind[j]==CRS_colind[new_curr-1]) {
        CRS_vals[new_curr-1] += CRS_vals[j];
      }
      else if(new_curr==j) {
        new_curr++;
      }
      else {
        CRS_colind[new_curr] = CRS_colind[j];
        CRS_vals[new_curr]   = CRS_vals[j];
        new_curr++;
      }
    }
    old_curr=new_curr;
  }

  CRS_rowptr[NumRows] = new_curr;
}

template <typename LocalOrdinal, typename GlobalOrdinal, typename Node>
void
lowCommunicationMakeColMapAndReindex (const Teuchos::ArrayView<const size_t> &rowptr,
                                      const Teuchos::ArrayView<LocalOrdinal> &colind_LID,
                                      const Teuchos::ArrayView<GlobalOrdinal> &colind_GID,
                                      const Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> >& domainMapRCP,
                                      const Teuchos::ArrayView<const int> &owningPIDs,
                                      Teuchos::Array<int> &remotePIDs,
                                      Teuchos::RCP<const Tpetra::Map<LocalOrdinal,GlobalOrdinal,Node> > & colMap)
{
  using Teuchos::rcp;
  typedef LocalOrdinal LO;
  typedef GlobalOrdinal GO;
  typedef Tpetra::global_size_t GST;
  typedef Tpetra::Map<LO, GO, Node> map_type;
  const char prefix[] = "lowCommunicationMakeColMapAndReindex: ";

  // The domainMap is an RCP because there is a shortcut for a
  // (common) special case to return the columnMap = domainMap.
  const map_type& domainMap = *domainMapRCP;

  // Scan all column indices and sort into two groups:
  // Local:  those whose GID matches a GID of the domain map on this processor and
  // Remote: All others.
  const size_t numDomainElements = domainMap.getNodeNumElements ();
  Teuchos::Array<bool> LocalGIDs;
  if (numDomainElements > 0) {
    LocalGIDs.resize (numDomainElements, false); // Assume domain GIDs are not local
  }

  // In principle it is good to have RemoteGIDs and RemotGIDList be as
  // long as the number of remote GIDs on this processor, but this
  // would require two passes through the column IDs, so we make it
  // the max of 100 and the number of block rows.
  //
  // FIXME (mfh 11 Feb 2015) Tpetra::Details::HashTable can hold at
  // most INT_MAX entries, but it's possible to have more rows than
  // that (if size_t is 64 bits and int is 32 bits).
  const size_t numMyRows = rowptr.size () - 1;
  const int hashsize = std::max (static_cast<int> (numMyRows), 100);

  Tpetra::Details::HashTable<GO, LO> RemoteGIDs (hashsize);
  Teuchos::Array<GO> RemoteGIDList;
  RemoteGIDList.reserve (hashsize);
  Teuchos::Array<int> PIDList;
  PIDList.reserve (hashsize);

  // Here we start using the *LocalOrdinal* colind_LID array.  This is
  // safe even if both columnIndices arrays are actually the same
  // (because LocalOrdinal==GO).  For *local* GID's set
  // colind_LID with with their LID in the domainMap.  For *remote*
  // GIDs, we set colind_LID with (numDomainElements+NumRemoteColGIDs)
  // before the increment of the remote count.  These numberings will
  // be separate because no local LID is greater than
  // numDomainElements.

  size_t NumLocalColGIDs = 0;
  LO NumRemoteColGIDs = 0;
  for (size_t i = 0; i < numMyRows; ++i) {
    for(size_t j = rowptr[i]; j < rowptr[i+1]; ++j) {
      const GO GID = colind_GID[j];
      // Check if GID matches a row GID
      const LO LID = domainMap.getLocalElement (GID);
      if(LID != -1) {
        const bool alreadyFound = LocalGIDs[LID];
        if (! alreadyFound) {
          LocalGIDs[LID] = true; // There is a column in the graph associated with this domain map GID
          NumLocalColGIDs++;
        }
        colind_LID[j] = LID;
      }
      else {
        const LO hash_value = RemoteGIDs.get (GID);
        if (hash_value == -1) { // This means its a new remote GID
          const int PID = owningPIDs[j];
          TEUCHOS_TEST_FOR_EXCEPTION(
            PID == -1, std::invalid_argument, prefix << "Cannot figure out if "
            "PID is owned.");
          colind_LID[j] = static_cast<LO> (numDomainElements + NumRemoteColGIDs);
          RemoteGIDs.add (GID, NumRemoteColGIDs);
          RemoteGIDList.push_back (GID);
          PIDList.push_back (PID);
          NumRemoteColGIDs++;
        }
        else {
          colind_LID[j] = static_cast<LO> (numDomainElements + hash_value);
        }
      }
    }
  }

  // Possible short-circuit: If all domain map GIDs are present as
  // column indices, then set ColMap=domainMap and quit.
  if (domainMap.getComm ()->getSize () == 1) {
    // Sanity check: When there is only one process, there can be no
    // remoteGIDs.
    TEUCHOS_TEST_FOR_EXCEPTION(
      NumRemoteColGIDs != 0, std::runtime_error, prefix << "There is only one "
      "process in the domain Map's communicator, which means that there are no "
      "\"remote\" indices.  Nevertheless, some column indices are not in the "
      "domain Map.");
    if (static_cast<size_t> (NumLocalColGIDs) == numDomainElements) {
      // In this case, we just use the domainMap's indices, which is,
      // not coincidently, what we clobbered colind with up above
      // anyway.  No further reindexing is needed.
      colMap = domainMapRCP;
      return;
    }
  }

  // Now build the array containing column GIDs
  // Build back end, containing remote GIDs, first
  const LO numMyCols = NumLocalColGIDs + NumRemoteColGIDs;
  Teuchos::Array<GO> ColIndices;
  GO* RemoteColIndices = NULL;
  if (numMyCols > 0) {
    ColIndices.resize (numMyCols);
    if (NumLocalColGIDs != static_cast<size_t> (numMyCols)) {
      RemoteColIndices = &ColIndices[NumLocalColGIDs]; // Points to back half of ColIndices
    }
  }

  for (LO i = 0; i < NumRemoteColGIDs; ++i) {
    RemoteColIndices[i] = RemoteGIDList[i];
  }

  // Build permute array for *remote* reindexing.
  Teuchos::Array<LO> RemotePermuteIDs (NumRemoteColGIDs);
  for (LO i = 0; i < NumRemoteColGIDs; ++i) {
    RemotePermuteIDs[i]=i;
  }

  // Sort External column indices so that all columns coming from a
  // given remote processor are contiguous.  This is a sort with two
  // auxilary arrays: RemoteColIndices and RemotePermuteIDs.
  Tpetra::sort3 (PIDList.begin (), PIDList.end (),
                 ColIndices.begin () + NumLocalColGIDs,
                 RemotePermuteIDs.begin ());

  // Stash the RemotePIDs.
  //
  // Note: If Teuchos::Array had a shrink_to_fit like std::vector,
  // we'd call it here.
  remotePIDs = PIDList;

  // Sort external column indices so that columns from a given remote
  // processor are not only contiguous but also in ascending
  // order. NOTE: I don't know if the number of externals associated
  // with a given remote processor is known at this point ... so I
  // count them here.

  // NTS: Only sort the RemoteColIndices this time...
  LO StartCurrent = 0, StartNext = 1;
  while (StartNext < NumRemoteColGIDs) {
    if (PIDList[StartNext]==PIDList[StartNext-1]) {
      StartNext++;
    }
    else {
      Tpetra::sort2 (ColIndices.begin () + NumLocalColGIDs + StartCurrent,
                     ColIndices.begin () + NumLocalColGIDs + StartNext,
                     RemotePermuteIDs.begin () + StartCurrent);
      StartCurrent = StartNext;
      StartNext++;
    }
  }
  Tpetra::sort2 (ColIndices.begin () + NumLocalColGIDs + StartCurrent,
                 ColIndices.begin () + NumLocalColGIDs + StartNext,
                 RemotePermuteIDs.begin () + StartCurrent);

  // Reverse the permutation to get the information we actually care about
  Teuchos::Array<LO> ReverseRemotePermuteIDs (NumRemoteColGIDs);
  for (LO i = 0; i < NumRemoteColGIDs; ++i) {
    ReverseRemotePermuteIDs[RemotePermuteIDs[i]] = i;
  }

  // Build permute array for *local* reindexing.
  bool use_local_permute = false;
  Teuchos::Array<LO> LocalPermuteIDs (numDomainElements);

  // Now fill front end. Two cases:
  //
  // (1) If the number of Local column GIDs is the same as the number
  //     of Local domain GIDs, we can simply read the domain GIDs into
  //     the front part of ColIndices, otherwise
  //
  // (2) We step through the GIDs of the domainMap, checking to see if
  //     each domain GID is a column GID.  we want to do this to
  //     maintain a consistent ordering of GIDs between the columns
  //     and the domain.
  Teuchos::ArrayView<const GO> domainGlobalElements = domainMap.getNodeElementList();
  if (static_cast<size_t> (NumLocalColGIDs) == numDomainElements) {
    if (NumLocalColGIDs > 0) {
      // Load Global Indices into first numMyCols elements column GID list
      std::copy (domainGlobalElements.begin (), domainGlobalElements.end (),
                 ColIndices.begin ());
    }
  }
  else {
    LO NumLocalAgain = 0;
    use_local_permute = true;
    for (size_t i = 0; i < numDomainElements; ++i) {
      if (LocalGIDs[i]) {
        LocalPermuteIDs[i] = NumLocalAgain;
        ColIndices[NumLocalAgain++] = domainGlobalElements[i];
      }
    }
    TEUCHOS_TEST_FOR_EXCEPTION(
      static_cast<size_t> (NumLocalAgain) != NumLocalColGIDs,
      std::runtime_error, prefix << "Local ID count test failed.");
  }

  // Make column Map
  const GST minus_one = Teuchos::OrdinalTraits<GST>::invalid ();
  colMap = rcp (new map_type (minus_one, ColIndices, domainMap.getIndexBase (),
                              domainMap.getComm (), domainMap.getNode ()));

  // Low-cost reindex of the matrix
  for (size_t i = 0; i < numMyRows; ++i) {
    for (size_t j = rowptr[i]; j < rowptr[i+1]; ++j) {
      const LO ID = colind_LID[j];
      if (static_cast<size_t> (ID) < numDomainElements) {
        if (use_local_permute) {
          colind_LID[j] = LocalPermuteIDs[colind_LID[j]];
        }
        // In the case where use_local_permute==false, we just copy
        // the DomainMap's ordering, which it so happens is what we
        // put in colind_LID to begin with.
      }
      else {
        colind_LID[j] =  NumLocalColGIDs + ReverseRemotePermuteIDs[colind_LID[j]-numDomainElements];
      }
    }
  }
}

} // namespace Import_Util
} // namespace Tpetra

// We can include the definitions for Tpetra::CrsMatrix now that the above
// functions have been defined.  For ETI, this isn't necessary, so we just
// including the generated hpp
#include "Tpetra_CrsMatrix.hpp"

#endif // TPETRA_IMPORT_UTIL_HPP
