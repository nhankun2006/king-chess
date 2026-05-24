#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <optional>
#include <vector>

#include "chess/model/Move.h"

/// Network protocol for LAN PvP chess.
///
/// Wire format (little-endian):
///   [1 byte MessageType][2 bytes payload length][N bytes payload]
///
/// This gives a fixed 3-byte header followed by a variable-length payload.
/// For a Move message the payload is 7 bytes (see encodeMovePayload).
namespace Protocol {

/// Header size: 1 byte type + 2 bytes length = 3 bytes.
constexpr size_t kHeaderSize = 3;

/// Maximum payload size we'll accept (guard against malformed data).
constexpr size_t kMaxPayloadSize = 256;

enum class MessageType : uint8_t {
  Move = 0x01,    // A chess move
  Restart = 0x02, // Restart request
  Resign = 0x03,  // Player resigned
  Ping = 0x10,    // Keep-alive
  Pong = 0x11,    // Keep-alive response
};

/// A decoded network message.
struct Message {
  MessageType type;
  std::vector<uint8_t> payload;
};

// ── Move serialization ──────────────────────────────────────────────────────

/// Encode a Move into a payload byte vector.
/// Layout: [fromRow(1)][fromCol(1)][toRow(1)][toCol(1)]
///         [promotionType(1)][isCastling(1)][isEnPassant(1)] = 7 bytes.
std::vector<uint8_t> encodeMovePayload(const Move &move);

/// Decode a Move from a payload byte vector.
/// Returns std::nullopt if the payload is malformed.
std::optional<Move> decodeMovePayload(const std::vector<uint8_t> &payload);

// ── Full message encode/decode ──────────────────────────────────────────────

/// Encode a full wire message (header + payload).
std::vector<uint8_t> encodeMessage(MessageType type,
                                   const std::vector<uint8_t> &payload = {});

/// Convenience: encode a Move as a complete wire message.
std::vector<uint8_t> encodeMoveMessage(const Move &move);

/// Try to decode one complete message from a byte buffer.
/// On success, returns the Message and sets `bytesConsumed` to the number
/// of bytes used from `data`.  Returns std::nullopt if the buffer does not
/// yet contain a complete message (caller should accumulate more data).
std::optional<Message> decodeMessage(const uint8_t *data, size_t length,
                                     size_t &bytesConsumed);

} // namespace Protocol

#endif // PROTOCOL_H
