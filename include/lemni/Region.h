/*
	The Lemni Programming Language - Functional computer speak
	Copyright (C) 2020  Keith Hammond

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LEMNI_REGION_H
#define LEMNI_REGION_H 1

/**
 * @defgroup Regions Region-based memory management
 * @{
 */

#include "Interop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LemniRegionT *LemniRegion;
typedef const struct LemniRegionT *LemniRegionConst;

typedef struct LemniStorageT *LemniStorage;
typedef const struct LemniStorageT *LemniStorageConst;

/**
 * @brief Create a memory region.
 * @warning if \p parent is ``NULL`` the region must be destroyed with \ref lemniDestroyRegion .
 * @param parent parent of the region to create, or ``NULL`` for no parent
 * @returns newly created region
 */
LemniRegion lemniCreateRegion(LemniRegion parent);

/**
 * @brief Destroy a region previously created with \ref lemniCreateRegion .
 * @param region the region to destroy
 */
void lemniDestroyRegion(LemniRegion region);

/**
 * @brief Get the parent of a region.
 * @param region region to query
 * @returns the parent of \p region or ``NULL`` if no parent exists
 */
LemniRegionConst lemniRegionParent(LemniRegionConst region);

/**
 * @brief Get the number of children in a region.
 * @param region region to query
 * @returns number of children in \p region
 */
LemniNat64 lemniRegionNumChildren(LemniRegionConst region);

/**
 * @brief Get a child region
 * @param region the region to query
 * @param idx index of the child
 * @returns the child or ``NULL`` if \p idx out of bounds
 * @note see \ref lemniRegionNumChildren for upper bound.
 */
LemniRegionConst lemniRegionChild(LemniRegionConst region, const LemniNat64 idx);

/**
 * @brief Get number of storage spaces owned by a region.
 * @param region region to query
 * @returns number of storage spaces
 */
LemniNat64 lemniRegionNumStorages(LemniRegionConst region);

/**
 * @brief Get the minimum possible alignment of a region
 * @param region region to query
 * @param queryChildren whether to include children in alignment calculation
 * @returns alignment in bytes
 */
LemniNat64 lemniRegionMinAlignment(LemniRegionConst region, const bool queryChildren);

/**
 * @brief Get the raw/packed size of a region.
 * @param region region to query
 * @param queryChildren whether to include children in size calculation
 * @returns size in bytes
 */
LemniNat64 lemniRegionRawSize(LemniRegionConst region, const bool queryChildren);

/**
 * @brief Get the aligned size of a region.
 * @param region region to query
 * @param queryChildren whether to include children in size calculation
 * @param alignment minimum alignment to use (pass 0 to use the minimum alignment of all allocated storage)
 * @returns size in bytes
 */
LemniNat64 lemniRegionAlignedSize(LemniRegionConst region, const bool queryChildren, const LemniNat64 alignment);

typedef struct LemniMemorySizeT{
	LemniNat64 raw, aligned;
} LemniMemorySize;

/**
 * @brief Get both the raw and aligned size of a region.
 * @param region region to query
 * @param queryChildren whether to include children in the calculation
 * @param alignment minimum alignment to use (pass 0 to use the minimum alignment of all allocated storage)
 * @returns both the raw and aligned size
 */
LemniMemorySize lemniRegionSize(LemniRegionConst region, const bool queryChildren, const LemniNat64 alignment);

/**
 * @brief Get allocated storage owned by a region.
 * @param region region to query
 * @param idx index of the storage space
 * @returns storage space or ``NULL`` if \p idx out of bounds
 * @note see \ref lemniRegionNumStorages for upper bound.
 */
LemniStorageConst lemniRegionStorage(LemniRegionConst region, const LemniNat64 idx);

/**
 * @brief Allocate sized storage inside a region.
 * @warning storage will be deallocated when the owning region is destroyed. see \ref lemniStoragePreserve .
 * @param region region to allocate memory from
 * @param size size in bytes of requested storage
 * @param alignment alignment to use (rounded up to nearest power of 2). 0 indicates tightly packed data
 * @returns the newly allocated storage
 */
LemniStorage lemniRegionAlloc(LemniRegion region, const LemniNat64 size, const LemniNat64 alignment);

/**
 * @brief Get the raw size (in bytes) of some storage.
 * @param storage the storage to query
 * @returns the raw size of the storage, or 0 for a ``NULL`` \p storage .
 */
LemniNat64 lemniStorageRawSize(LemniStorageConst storage);

/**
 * @brief Get the aligned size (in bytes) of some storage.
 * @param storage the storage to query
 * @returns the aligned size of the storage, or 0 for a ``NULL`` \p storage .
 */
LemniNat64 lemniStorageAlignedSize(LemniStorageConst storage);

/**
 * @brief Get both the raw and aligned size (in bytes) of some storage.
 * @param storage the storage to query
 * @returns both the raw and aligned size
 */
LemniMemorySize lemniStorageSize(LemniStorageConst storage);

/**
 * @brief Get the alignment (in bytes) of some storage.
 * @param storage the storage to query
 * @returns storage size in bytes, or 0 for a ``NULL`` \p storage .
 */
LemniNat64 lemniStorageAlignment(LemniStorageConst storage);

/**
 * @brief Set the owner of some storage to the parent n levels above it's current owner.
 * @param storage the storage to alter
 * @param levels number of parent regions above current owner to set as owner
 * @returns the actual number of parents that ownership was raised by, 0 if there are no parent regions
 */
LemniNat64 lemniStoragePreserve(LemniStorage storage, const LemniNat64 levels);

/**
 * @brief Transfer ownership of a storage to a region.
 * @param storage the storage to alter
 * @param region the region to transfer ownership to
 * @returns whether the operation was successful
 */
bool lemniStorageTransfer(LemniStorage storage, LemniRegion region);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !LEMNI_REGION_H
