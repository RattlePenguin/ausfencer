# Some installed Crow headers default-construct request connection flags without
# initializing them, then move that request in HTTPParser::clear(). Keep the
# installed dependency read-only and apply the narrow fix to a build-local copy.
set(CROW_COMPAT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/crow-compat")
file(COPY "${CROW_INCLUDE_DIR}/crow" "${CROW_INCLUDE_DIR}/crow.h"
  DESTINATION "${CROW_COMPAT_INCLUDE_DIR}")
set(crow_request_header "${CROW_COMPAT_INCLUDE_DIR}/crow/http_request.h")
file(READ "${crow_request_header}" crow_request_source)
string(REPLACE "method(HTTPMethod::Get)\n        {}"
  "method(HTTPMethod::Get),\n          http_ver_major{}, http_ver_minor{}, keep_alive{}, close_connection{}, upgrade{}\n        {}"
  crow_request_fixed "${crow_request_source}")
if(NOT crow_request_fixed STREQUAL crow_request_source)
  file(WRITE "${crow_request_header}" "${crow_request_fixed}")
  message(STATUS "Applied Crow default-request initialization compatibility fix")
endif()
