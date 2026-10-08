#pragma once

#include "models/Bout.hpp"
#include <sqlite_orm/sqlite_orm.h>
#include <stdexcept>
#include <string>

// sqlite_orm requires explicit traits for enum fields. Store the existing
// Weapon values as INTEGER while keeping the model field typed as Weapon.
namespace sqlite_orm {

template <> struct type_printer<Weapon> : integer_printer {};

template <> struct statement_binder<Weapon> {
  int bind(sqlite3_stmt *stmt, int index, const Weapon &value) const {
    return statement_binder<int>{}.bind(stmt, index, static_cast<int>(value));
  }
};

template <> struct field_printer<Weapon> {
  std::string operator()(const Weapon &value) const {
    return field_printer<int>{}(static_cast<int>(value));
  }
};

template <> struct row_extractor<Weapon> {
  Weapon extract(const char *text) const {
    return from_integer(row_extractor<long long>{}.extract(text));
  }

  Weapon extract(sqlite3_stmt *stmt, int column_index) const {
    return from_integer(row_extractor<long long>{}.extract(stmt, column_index));
  }

private:
  static Weapon from_integer(long long value) {
    switch (value) {
    case Foil:
      return Foil;
    case Epee:
      return Epee;
    case Sabre:
      return Sabre;
    default:
      throw std::domain_error("Invalid stored Weapon value");
    }
  }
};

} // namespace sqlite_orm
