// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Tests v8::internal::Scanner. Note that presently most unit tests for the
// Scanner are in cctest/test-parsing.cc, rather than here.

#include "src/handles-inl.h"
#include "src/parsing/scanner-character-streams.h"
#include "src/parsing/scanner.h"
#include "src/unicode-cache.h"
#include "test/cctest/cctest.h"

using namespace v8::internal;

namespace {

const char src_simple[] = "function foo() { var x = 2 * a() + b; }";

std::unique_ptr<Scanner> make_scanner(const char* src) {
  std::unique_ptr<Scanner> scanner(new Scanner(new UnicodeCache()));
  scanner->Initialize(ScannerStream::ForTesting(src).release());
  return scanner;
}

}  // anonymous namespace

TEST(Bookmarks) {
  // Scan through the given source and record the tokens for use as reference
  // below.
  std::vector<Token::Value> tokens;
  {
    auto scanner = make_scanner(src_simple);
    do {
      tokens.push_back(scanner->Next());
    } while (scanner->current_token() != Token::EOS);
  }

  // For each position:
  // - Scan through file,
  // - set a bookmark once the position is reached,
  // - scan a bit more,
  // - reset to the bookmark, and
  // - scan until the end.
  // At each step, compare to the reference token sequence generated above.
  for (size_t bookmark_pos = 0; bookmark_pos < tokens.size(); bookmark_pos++) {
    auto scanner = make_scanner(src_simple);
    Scanner::BookmarkScope bookmark(scanner.get());

    for (size_t i = 0; i < std::min(bookmark_pos + 10, tokens.size()); i++) {
      if (i == bookmark_pos) {
        bookmark.Set();
      }
      DCHECK_EQ(tokens[i], scanner->Next());
    }

    bookmark.Apply();
    for (size_t i = bookmark_pos; i < tokens.size(); i++) {
      DCHECK_EQ(tokens[i], scanner->Next());
    }
  }
}
