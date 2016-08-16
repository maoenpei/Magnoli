#include "LargeIntegerData.h"

#include "MemUtil.h"
#include "TlsUtil.h"
#include <assert.h>
#include <algorithm>
#include <map>
#include <vector>

#define ASSERT_INT(EXP, ...)        assert(EXP)

namespace mag
{
    struct BlockManager
    {
        std::unique_ptr<util::MemUtil> mem;
        std::vector<LargeIntegerData::IntegerType*> nilBlocks;
        std::vector<LargeIntegerData::IntegerType*>::size_type nilCount;
        int blockSize;

        BlockManager(int blockSize)
            : blockSize(blockSize)
            , nilCount(0)
        {
            mem = std::make_unique<util::MemUtil>(SIZ(blockSize) * std::max(256 / blockSize, 4));
        }

        LargeIntegerData::IntegerType* query() {
            if (nilCount == 0) {
                return static_cast<LargeIntegerData::IntegerType*>(mem->query(SIZ(blockSize)));
            } else {
                -- nilCount;
                return nilBlocks[nilCount];
            }
        }

        void returnBack(LargeIntegerData::IntegerType* ptr) {
            if (nilCount == nilBlocks.size()) {
                nilBlocks.push_back(ptr);
            } else {
                nilBlocks[nilCount] = ptr;
            }
            ++ nilCount;
        }
    };

    struct MemAllocator
    {
        std::map<int, std::shared_ptr<BlockManager>> blockManagers;
        std::unique_ptr<util::MemUtil> cache;

        MemAllocator() {
            cache = std::make_unique<util::MemUtil>(SIZ(4));
        }

        ~MemAllocator() {
        }

        util::MemUtil& GetCache() {
            return *cache;
        }

        BlockManager& GetBlockManager(int blockSize) {
            if (blockManagers.find(blockSize) == blockManagers.end()) {
                blockManagers.insert(std::make_pair(
                    blockSize,
                    std::make_shared<BlockManager>(blockSize)));
            }
            return *blockManagers[blockSize];
        }

        static MemAllocator& current() {
            return *reinterpret_cast<MemAllocator*>(tls.get());
        }
    private:
        static util::TlsUtil tls;
    };

    util::TlsUtil MemAllocator::tls(
        [](){return reinterpret_cast<void*>(new MemAllocator());},
        [](void* ptr){delete reinterpret_cast<MemAllocator*>(ptr);}
    );

    LargeIntegerDataStorage::LargeIntegerDataStorage()
        : size(0)
        , threadEnv(&MemAllocator::current())
    {
        reset(1);
    }

    LargeIntegerDataStorage::LargeIntegerDataStorage(const LargeIntegerDataStorage& copy)
        : LargeIntegerDataStorage()
    {
        ASSERT_INT(threadEnv == copy.threadEnv);
        reset(copy);
    }

    LargeIntegerDataStorage::~LargeIntegerDataStorage()
    {
        if (size) {
            ASSERT_INT(threadEnv == reinterpret_cast<void*>(&MemAllocator::current()));
            MemAllocator::current().GetBlockManager(size).returnBack(data);
        }
    }

    void LargeIntegerDataStorage::alloc(int dim)
    {
        if (dim > size) {
            ASSERT_INT(threadEnv == reinterpret_cast<void*>(&MemAllocator::current()));
            if (size) {
                MemAllocator::current().GetBlockManager(size).returnBack(data);
            }
            size = dim;
            LargeIntegerData::data = MemAllocator::current().GetBlockManager(size).query();
        }
    }

    LargeIntegerDataTmp& LargeIntegerDataTmp::swap(LargeIntegerDataTmp&& copy)
    {
        std::swap(data, copy.data);
        std::swap(dim, copy.dim);
        return *this;
    }

    void LargeIntegerDataTmp::clearCache()
    {
        MemAllocator::current().GetCache().reset();
    }

    void LargeIntegerDataTmp::alloc(int dim)
    {
        LargeIntegerData::data = static_cast<LargeIntegerData::IntegerType*>(MemAllocator::current().GetCache().query(SIZ(dim)));
    }
}
