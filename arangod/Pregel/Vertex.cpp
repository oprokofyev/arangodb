////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#include "Vertex.h"
#include "OutMessageCache.h"

#include "Basics/StaticStrings.h"
#include <velocypack/velocypack-aliases.h>


using namespace std;
using namespace arangodb;
using namespace arangodb::velocypack;
using namespace arangodb::pregel;

Message::Message(VPackSlice slice) {
  VPackSlice s = slice.get("value");
  _value = s.getSmallInt() || s.getInt() ? s.getInt() : -1;
}

Vertex::Vertex(VPackSlice document) : _data(document)  {
  //documentId = document.get(StaticStrings::IdString).copyString();
  VPackSlice s = document.get("value");
  _vertexState = s.isInteger() ? s.getInt() : -1;
}

Vertex::~Vertex() {
  /*for (auto const &it : _edges) {
    delete(it);
  }*/
  _edges.clear();
}

void Vertex::compute(int gss, MessageIterator const &messages, OutMessageCache* const cache) {
  int current = _vertexState;
  for (auto const &msg : messages) {
    if (msg._value < current) current = msg._value;
  }
  if (current >= 0 && (gss == 0 || current != _vertexState))  {
    _vertexState = current;
    for (auto const &edge : _edges) {
      VPackSlice toID = edge._data.get(StaticStrings::ToString);
      VPackBuilder b;
      b.openObject();
      b.add(StaticStrings::ToString, toID);
      b.add("value", VPackValue(edge._value + current));
      b.close();
      cache->addMessage(toID.copyString(), b.slice());
    }
  } else voteHalt();
}