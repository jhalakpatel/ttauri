// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "file_view.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "memory.hpp"
#include "URL.hpp"
#include "required.hpp"
#include "unfair_mutex.hpp"
#include <mutex>

namespace tt {

std::shared_ptr<file_mapping> file_view::findOrCreateFileMappingObject(URL const& location, access_mode accessMode, size_t size)
{
    static unfair_mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<file_mapping>>> mappedFileObjects;

    ttlet lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFileObjects);

    auto& mappings = mappedFileObjects[location];

    for (auto weak_file_mapping_object : mappings) {
        if (auto _file_mapping_object = weak_file_mapping_object.lock()) {
            if (_file_mapping_object->size >= size && _file_mapping_object->accessMode() >= accessMode) {
                return _file_mapping_object;
            }
        }
    }

    auto _file_mapping_object = std::make_shared<file_mapping>(location, accessMode, size);
    mappings.push_back(_file_mapping_object);
    return _file_mapping_object;
}

}
