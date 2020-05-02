/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/time_util.h"

#include "base/i18n/time_formatting.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"

namespace ads {

std::string LongFormatFriendlyDateAndTime(
    const base::Time& time) {
  const base::string16 friendly_date_and_time =
      base::TimeFormatFriendlyDateAndTime(time);

  return base::UTF16ToUTF8(friendly_date_and_time);
}

std::string FriendlyDateAndTime(
    const base::Time& time) {
  const base::TimeDelta time_delta = time - base::Time::Now();

  if (time_delta.InDays() > 0) {
    // Show date and time as "Tuesday, 5 May 2020 at 18:27:34" if over 1 day
    return LongFormatFriendlyDateAndTime(time);
  }

  // Show date and time as "0 hours 21 minutes, 58 seconds"
  base::string16 time_duration;
  if (!base::TimeDurationFormatWithSeconds(time_delta,
      base::DURATION_WIDTH_WIDE, &time_duration)) {
    return LongFormatFriendlyDateAndTime(time);
  }

  return base::UTF16ToUTF8(time_duration);
}

std::string FriendlyDateAndTime(
    const uint64_t timestamp_in_seconds) {
  const base::Time time = base::Time::FromDoubleT(timestamp_in_seconds);
  return FriendlyDateAndTime(time);
}

uint64_t MigrateTimestampToDoubleT(
    const uint64_t timestamp_in_seconds) {
  if (timestamp_in_seconds < 10000000000) {
    // Already migrated as DoubleT will never reach 10000000000 in our lifetime
    // and legacy timestamps are above 10000000000
    return timestamp_in_seconds;
  }

  // Migrate date to DoubleT
  const base::Time now = base::Time::Now();

  const uint64_t now_in_seconds =
      static_cast<uint64_t>((now - base::Time()).InSeconds());

  const uint64_t delta = timestamp_in_seconds - now_in_seconds;

  const base::Time time = now + base::TimeDelta::FromSeconds(delta);
  return time.ToDoubleT();
}

}  // namespace ads
