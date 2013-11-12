/*
 * Copyright 2013 Matthew Harvey
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GUARD_id_hpp_3557065615224217
#define GUARD_id_hpp_3557065615224217

namespace sqloxx
{

/**
 * Type used for unique identifier for instances of sqloxx::PersistentObject.
 * This corresponds to the primary key of such types when persisted to
 * a database.
 */
typedef int Id;

}  // namespace sqloxx

#endif  // GUARD_id_hpp_3557065615224217
