#include "Ticket.hpp"
#include <winsock2.h> // Required for htonl to handle network byte ordering
#include <cstring>

namespace dnflogin::core {

std::vector<unsigned char> Ticket::CreateRawPacket(unsigned int uid) {
    // 1. Initialize an explicit 46-byte buffer pre-filled with the classic 0x01 routing padding
    std::vector<unsigned char> packet(46, 0x01);

    // 2. Format the dynamic Account ID block (Bytes 0 to 3)
    // Convert the unsigned int to Network Byte Order (Big-Endian) to ensure proper character reading
    unsigned int bigEndianUID = htonl(uid);
    std::memcpy(packet.data(), &bigEndianUID, 4);

    // 3. Keep Bytes 4 through 35 un-touched. They remain as the static 32-byte middleware padding of 0x01

    // 4. Inject the immutable verification trailer signature (Bytes 36 through 45)
    // If you alter even a single coordinate of this array, the client protocol engine fails
    const unsigned char serverTrailer[10] = { 
        0x55, 0x91, 0x45, 0x10, 0x01, 0x04, 0x03, 0x03, 0x01, 0x01 
    };
    std::memcpy(packet.data() + 36, serverTrailer, 10);

    return packet; // Yields the exact 46-byte block verified by your Kali decrypt runtime
}

} // namespace dnflogin::core
