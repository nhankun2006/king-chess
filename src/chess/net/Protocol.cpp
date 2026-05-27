#include "chess/net/Protocol.h"

#include <cstring>

namespace Protocol {

// ── Move serialization ──────────────────────────────────────────────────────

std::vector<uint8_t> encodeMovePayload(const Move &move) {
  std::vector<uint8_t> payload(7);
  payload[0] = static_cast<uint8_t>(move.from.row);
  payload[1] = static_cast<uint8_t>(move.from.col);
  payload[2] = static_cast<uint8_t>(move.to.row);
  payload[3] = static_cast<uint8_t>(move.to.col);
  payload[4] = static_cast<uint8_t>(move.promotion);
  payload[5] = move.isCastling ? 1 : 0;
  payload[6] = move.isEnPassant ? 1 : 0;
  return payload;
}

std::optional<Move> decodeMovePayload(const std::vector<uint8_t> &payload) {
  if (payload.size() < 7) return std::nullopt;

  // Validate PieceType range before casting
  uint8_t rawPromotion = payload[4];
  if (rawPromotion > static_cast<uint8_t>(PieceType::King)) return std::nullopt;

  Move move;
  move.from.row = static_cast<int>(payload[0]);
  move.from.col = static_cast<int>(payload[1]);
  move.to.row = static_cast<int>(payload[2]);
  move.to.col = static_cast<int>(payload[3]);
  move.promotion = static_cast<PieceType>(rawPromotion);
  move.isCastling = (payload[5] != 0);
  move.isEnPassant = (payload[6] != 0);

  // Basic validation: positions must be within the 8x8 board
  if (!move.from.isValid() || !move.to.isValid()) return std::nullopt;

  return move;
}

// ── Full message encode/decode ──────────────────────────────────────────────

std::vector<uint8_t> encodeMessage(MessageType type,
                                   const std::vector<uint8_t> &payload) {
  // Guard against payload sizes that would overflow the 2-byte length field
  if (payload.size() > kMaxPayloadSize) return {};

  uint16_t payloadLen = static_cast<uint16_t>(payload.size());

  std::vector<uint8_t> msg;
  msg.reserve(kHeaderSize + payload.size());

  // Header: [type(1)][length_lo(1)][length_hi(1)]
  msg.push_back(static_cast<uint8_t>(type));
  msg.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
  msg.push_back(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));

  // Payload
  msg.insert(msg.end(), payload.begin(), payload.end());

  return msg;
}

std::vector<uint8_t> encodeMoveMessage(const Move &move) {
  return encodeMessage(MessageType::Move, encodeMovePayload(move));
}

/// Check whether a raw byte corresponds to a known MessageType value.
static bool isValidMessageType(uint8_t raw) {
  switch (static_cast<MessageType>(raw)) {
  case MessageType::Move:
  case MessageType::Restart:
  case MessageType::Resign:
  case MessageType::Ping:
  case MessageType::Pong:
    return true;
  default:
    return false;
  }
}

std::optional<Message> decodeMessage(const uint8_t *data, size_t length,
                                     size_t &bytesConsumed) {
  bytesConsumed = 0;

  // Need at least the header
  if (length < kHeaderSize) return std::nullopt;

  // Validate message type before casting
  if (!isValidMessageType(data[0])) return std::nullopt;

  MessageType type = static_cast<MessageType>(data[0]);
  uint16_t payloadLen =
      static_cast<uint16_t>(data[1]) | (static_cast<uint16_t>(data[2]) << 8);

  // Guard against absurdly large payloads
  if (payloadLen > kMaxPayloadSize) return std::nullopt;

  // Need header + full payload
  size_t totalSize = kHeaderSize + payloadLen;
  if (length < totalSize) return std::nullopt;

  Message msg;
  msg.type = type;
  msg.payload.assign(data + kHeaderSize, data + totalSize);

  bytesConsumed = totalSize;
  return msg;
}

} // namespace Protocol
