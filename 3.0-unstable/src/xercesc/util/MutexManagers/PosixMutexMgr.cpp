/*
 * Copyright 1999-2000,2004-2005 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: MacAbstractFile.hpp 176026 2004-09-08 13:57:07Z peiyongz $
 */

#include <xercesc/util/MutexManagers/PosixMutexMgr.hpp>

XERCES_CPP_NAMESPACE_BEGIN

PosixMutexMgr::PosixMutexMgr()
{
}


virtual
PosixMutexMgr::PosixMutexMgr()
{
}


virtual XMLMutexHandle
PosixMutexMgr::create(MemoryManager* const manager)
{
}


virtual void
PosixMutexMgr::destroy(XMLMutexHandle mtx, MemoryManager* const manager)
{
}


virtual void
PosixMutexMgr::lock(XMLMutexHandle mtx)
{
}


virtual void
PosixMutexMgr::unlock(XMLMutexHandle mtx)
{
}


XERCES_CPP_NAMESPACE_END


#endif