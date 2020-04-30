/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_PUBLISHER_V4_PUBLISHER_H_
#define BRAVELEDGER_PUBLISHER_V4_PUBLISHER_H_

namespace bat_ledger {
class LedgerImpl;
}

namespace braveledger_v4_publisher {

void FetchPrefixList(bat_ledger::LedgerImpl* ledger);

}  // namespace braveledger_publisher

#endif  // BRAVELEDGER_PUBLISHER_V4_PUBLISHER_H_
