#pragma once

namespace DeserializationError {
namespace Code {
enum {
    Ok,
    EmptyInput,
    IncompleteInput,
    InvalidInput,
    NoMemory,
    TooDeep,
};
}
} // namespace DeserializationError
