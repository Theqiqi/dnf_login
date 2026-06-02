#pragma once
#include <vector>

namespace dnflogin::core {

class Ticket {
public:
    // Disallow instantiation since this acts purely as a static utility engine
    Ticket() = delete;

    /**
     * @brief Constructs the exact 46-byte binary payload expected by the client.
     * @param uid The verified unique Account ID from the database.
     * @return A raw byte vector exactly 46 bytes in size.
     */
    static std::vector<unsigned char> CreateRawPacket(unsigned int uid);
};

} // namespace dnflogin::core
