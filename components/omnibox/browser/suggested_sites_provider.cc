/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/omnibox/browser/suggested_sites_provider.h"

#include <algorithm>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "components/omnibox/browser/autocomplete_input.h"

// As from autocomplete_provider.h:
// Search Secondary Provider (suggestion) |  100++
const int SuggestedSitesProvider::kRelevance = 100;


SuggestedSitesProvider::SuggestedSitesProvider(
    AutocompleteProviderClient* client)
    : AutocompleteProvider(AutocompleteProvider::TYPE_SEARCH) {
}

void SuggestedSitesProvider::Start(const AutocompleteInput& input,
                            bool minimal_changes) {
  matches_.clear();
  if (input.from_omnibox_focus() ||
      (input.type() == metrics::OmniboxInputType::EMPTY) ||
      (input.type() == metrics::OmniboxInputType::QUERY)) {
    return;
  }

  const std::string input_text =
      base::ToLowerASCII(base::UTF16ToUTF8(input.text()));
  auto check_add_match =
      [&](const std::pair<std::string, SuggestedSitesMatch>& pair) {
    const std::string& current_site = pair.first;
    const SuggestedSitesMatch& match = pair.second;
    // Don't bother matching until 4 chars, or less if it's an exact match
    if (input_text.length() < 4 &&
        current_site.length() != input_text.length()) {
      return;
    }
    size_t foundPos = current_site.find(input_text);
    // We'd normally check for npos here but we want only people that
    // really want these suggestions. Example don't suggest bitcoin and
    // litecoin for just a coin search.
    if (foundPos == 0) {
      ACMatchClassifications styles =
          StylesForSingleMatch(input_text,
              base::UTF16ToASCII(match.display_));
      AddMatch(base::ASCIIToUTF16(current_site), match, styles);
      if (match.allow_default_ &&
          current_site.length() == input_text.length()) {
        // It's guaranteed that matches_ has at least 1 item
        // here because of the previous AddMatch call.
        size_t last_index = matches_.size() - 1;
        matches_[last_index].SetAllowedToBeDefault(input);
        // As from autocomplete_provider.h:
        // Search Primary Provider (what you typed) | 1300
        matches_[last_index].relevance = 1301;
      }
    }
  };

  const auto& suggested_sites = GetSuggestedSites();
  std::for_each(suggested_sites.begin(), suggested_sites.end(),
      check_add_match);
}

SuggestedSitesProvider::~SuggestedSitesProvider() {}

// static
ACMatchClassifications SuggestedSitesProvider::StylesForSingleMatch(
    const std::string &input_text,
    const std::string &site) {
  ACMatchClassifications styles;
  size_t foundPos = site.find(input_text);
  if (std::string::npos == foundPos) {
    styles.push_back(ACMatchClassification(0, ACMatchClassification::NONE));
  } else if (foundPos == 0) {
    styles.push_back(ACMatchClassification(
        0, ACMatchClassification::URL | ACMatchClassification::MATCH));
    if (site.length() > input_text.length()) {
      styles.push_back(ACMatchClassification(input_text.length(),
                                             ACMatchClassification::URL));
    }
  } else {
    styles.push_back(ACMatchClassification(0, ACMatchClassification::URL));
    styles.push_back(ACMatchClassification(
        foundPos, ACMatchClassification::URL | ACMatchClassification::MATCH));
    if (site.length() > foundPos + input_text.length()) {
      styles.push_back(
          ACMatchClassification(foundPos + input_text.length(), 0));
    }
  }
  return styles;
}

void SuggestedSitesProvider::AddMatch(const base::string16& match_string,
                               const SuggestedSitesMatch& data,
                               const ACMatchClassifications& styles) {
  AutocompleteMatch match(this, kRelevance + matches_.size(), false,
                          AutocompleteMatchType::NAVSUGGEST);
  match.fill_into_edit = data.display_;
  match.destination_url = data.destination_url_;
  match.contents = data.display_;
  match.contents_class = styles;
  match.stripped_destination_url = data.stripped_destination_url_;
  matches_.push_back(match);
}
