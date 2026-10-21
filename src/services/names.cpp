#include "services/names.hpp"
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <stdexcept>

std::string fold_name(const std::string &input) {
  UErrorCode status = U_ZERO_ERROR;
  auto normalizer = icu::Normalizer2::getNFKCCasefoldInstance(status);
  icu::UnicodeString output;
  if (U_FAILURE(status)) throw std::runtime_error("Unicode normalizer unavailable");
  normalizer->normalize(icu::UnicodeString::fromUTF8(input), output, status);
  if (U_FAILURE(status)) throw std::invalid_argument("Invalid Unicode name");
  std::string result;
  return output.toUTF8String(result);
}
