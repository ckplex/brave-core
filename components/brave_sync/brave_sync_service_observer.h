/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_SYNC_BRAVE_SYNC_SERVICE_OBSERVER_H_
#define BRAVE_COMPONENTS_BRAVE_SYNC_BRAVE_SYNC_SERVICE_OBSERVER_H_

namespace brave_sync {

class BraveSyncService;

class BraveSyncServiceObserver : public base::CheckedObserver {
 public:
  ~BraveSyncServiceObserver() override {}

  virtual void OnSyncSetupError(BraveSyncService* sync_service,
                                const std::string& error) {}
  virtual void OnSyncStateChanged(BraveSyncService* sync_service) {}
  virtual void OnHaveSyncWords(BraveSyncService* sync_service,
                               const std::string& sync_words) {}
};

} // namespace brave_sync

#endif // BRAVE_COMPONENTS_BRAVE_SYNC_BRAVE_SYNC_SERVICE_OBSERVER_H_
