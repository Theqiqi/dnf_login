# DNF Taiwan Launcher - Reverse Engineering & Cryptographic Breakdown

This document permanently logs the cryptographic forensics, byte-level data layout extractions, and 

## 1. Bidirectional Cryptographic Data Flow

The diagram below maps how user identification parameters are packed, manually padded, 



```mermaid
flowchart TD
    subgraph Launcher_Encryption [ 1. C# Client-Side Encryption Workflow ]
        A[Input: Integer Account UID] --> B["Convert to 8-Char Hex (String_0)<br>Example: '1' -> '00000001'"]
        B --> C["Prepend String_2 (RSA Header)<br>'1FFFFF...00'"]
        C --> D["Append String_3 (Static Trailer)<br>'010101...0155914510010403030101'"]
        D --> E["Form 512-Character Hex String_1<br>(Matches 256-Byte PKCS#1 Layout)"]
        E --> F["Raw Modular Exponentiation Math<br>c = m^d mod n"]
        F --> G["Convert Output BigInteger to Base64<br>(Pass as Argument to DNF.exe)"]
    end

    subgraph Security_Interception [2. Interception Layer]
        G -->|Intercept CLI Args| H["Encrypted Base64 Token String"]
    end

    subgraph Python_Decryption [3. Python Diagnostic Decryption Workflow]
        H --> I[base64.b64decode]
        I --> J{"Byte Length == 255?"}
        J -->|Yes| K["Prepend Zero Byte \x00<br>Align to 256-Byte RSA Boundary"]
        J -->|No| L["Keep Raw 256 Bytes"]
        K --> M["Raw Public Key Modular Math<br>m = c^e mod n"]
        L --> M
        M --> N["Expose PKCS#1 v1.5 Structure<br>00 01 FF FF ... 00 [46-Byte Payload]"]
        N --> O["Locate Splitting Null Byte \x00<br>Strip Leading RSA Padding Block"]
        O --> P["Isolate Clean 46-Byte Game Ticket"]
        P --> Q["Parse Index 0:4: Account UID<br>Parse Index 36:46: Version Check"]
    end

```

Use code with caution.



------

## 2. Decrypted Token Payload Layout (The 46-Byte Vector)

Once the raw modular math strips the standard RSA encryption layer, the 

```
+------------------+----------------------------------+------------------------------------+

| Bytes 0x00 - 0x03|        Bytes 0x04 - 0x23         |         Bytes 0x24 - 0x2D          |
|     (4 Bytes)    |            (32 Bytes)            |             (10 Bytes)             |
+------------------+----------------------------------+------------------------------------+

|   Account UID    |      Routing Cipher Padding      |      Version-Specific Trailer      |
|   (Big-Endian)   |     Pre-filled with 0x01 bytes   |    55 91 45 10 01 04 03 03 01 01   |
+------------------+----------------------------------+------------------------------------+
```

Detailed Field Specifications

| Byte Offset (Hex) | Byte Length | Hex Values / Pattern            | Structural Data Type     | Functional Mapping                                           |
| ----------------- | ----------- | ------------------------------- | ------------------------ | ------------------------------------------------------------ |
| `0x00 - 0x03`     | 4 Bytes     | E.g., `00 00 00 01`             | `uint32_t` (Big-Endian)  | **Account Unique Identifier**. Mapped directly out of the database primary cluster keys. |
| `0x04 - 0x23`     | 32 Bytes    | `01 01 01 ... 01`               | `uint8_t[]` (Byte Array) | **Routing Cipher Padding**. A fixed sequence deployed by the network engine to satisfy minimum boundary layouts. |
| `0x24 - 0x2D`     | 10 Bytes    | `55 91 45 10 01 04 03 03 01 01` | `uint8_t[]` (Byte Array) | **Client Checksum Trailer**. Static version token validated by the server runtime to grant channel clearance. |

------

## 3. Comparative Implementation Equivalence

The core engineering breakthrough of the modern C++ launcher was proving that **Manual Code-Level String Concatenation** and **Standardized API Padding Enforcements** yield mathematically identical binary blocks prior to modular exponentiation.

### Approach A: The Legacy C# Flawed Method

The legacy code generated a massive string by hand-stitching the padding parts together before computing the huge integer values:


```csharp

// Manual string building simulating PKCS#1 v1.5 padding structure
this.string_1 = this.string_2 (1FFFF...00) + this.string_0 (UID Hex) + this.string_3 (0101...Trailer);
BigInteger bigInteger = new BigInteger(this.string_1, 16);
// Multiplied manually inside an obscure custom big-number loops
```

Use code with caution.

### Approach B: Our Modern Decoupled C++ Method

Our system delegates structural alignment straight to verified cryptographic libraries. By enforcing **`RSA_PKCS1_PADDING`**, the low-level OpenSSL stack automates the generation of the exact `00 01 FF...FF 00` header layout inside protected memory arrays.

``` cpp
// 1. Ticket generates a pure, clean 46-byte layout block array
std::vector<unsigned char> rawPacket = core::Ticket::CreateRawPacket(targetUID);

// 2. Crypto instructs OpenSSL to wrap and auto-pad the block cleanly
int encryptedLen = RSA_private_encrypt(
    46, rawPacket.data(), encryptedData.data(), rsa, 
    RSA_PKCS1_PADDING // <--- Automatically bakes the exact 1FFFF...00 padding block!
);
```

### Approach A: The Legacy C# source string and privatekey

**string**

```csharp
	this.string_2 = "1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00";
	IL_BC:
	this.string_3 = "010101010101010101010101010101010101010101010101010101010101010155914510010403030101";
```

**privatekey**

```c#
RSAParameters rsaparameters = Form1.ConvertFromPemPrivateKey("-----BEGIN RSA PRIVATE KEY-----\r\nMIIEpQIBAAKCAQEA2Mzieb9pITcA4f8ZlcJ678aFWWwQURecoJCMUdA5Cg5X6w5/\r\numgvWmOgadWeZKTehHn08KIObx7L0x/4RerkZFYPdigu7fOoJWUle7i3PU2Ussyx\r\nQa56O5eAQDuQZGSuKKEqUWL7CFpXJUat0kYg8/6o3U9goWcu4jGokjrUTsrj0++9\r\n3mGssPAeB6NY+LOdkVlw4cQKm7EFmxXeMf3gdnD461PKP6XnlVaLliA+mlEeGpqU\r\nHfQ8S4RlnxGXLP47kgCeU2OA0T1MgRGV8PpduEI5G65TVZ7y7jQrNJZRj3ofVV0t\r\njB7fNqf/Gj/qyOKBYUWVrZGU2btRHejMVkOnIwIDAQABAoIBAQDTZ6aIFbBMJTiF\r\nJ54pPMVoPmsV8ZxPlviyUYGi3aphNe9hVHgUqzdRqshnq1iSx3n8MHg6lawBi0Qy\r\nEYClnREtDgZxr2ljuy8BmAnfBRYZfyc62wMWCy9CIM980xhP7SUulUmQpzYmxQEp\r\nZixlWOEVTAQaGicd/GHpS4cXYKpaUQ4h0RnjOSut72vR80Dpb2mIM9wkWLPoFO5j\r\ndAsZ4t/CAReO8GgnAPWWFJ1tmZ+wNJCbUN9hp38bUYlDErmkihGzAKGtsSlZHPO7\r\n1cJngaVaJW4igFpiAnkevEfb4kwj/OVQZLdYe1P3wLdkCK+QJu/VCfKC3VzTo+ee\r\nclrRDsnxAoGBAPA4jdxJNoxvOK8mabEiRnh79VEfLc3edOZ+rFK6wN7+S5gj37bd\r\nEnEg2pBpum1XohnObvCQ2UOBuDzKDrWg8WEx++HOdmh8gPXeiImPJhqw3zh2yubc\r\n7cfzNmo+jo2OJAwZXT4LNz/NZY1vBlJjbYdVaWnhaEZYEzU4u2ZMpYO3AoGBAOcK\r\ngCipawANkEO7HfCAZxeTx3f1+JCOH08wJL7agRM8ZA5MP6RwwqpqSNYDlOGjINg/\r\n05AOc6rEM+S+4f91O914qU+32Hf1VqAp65N8zfgaiB6yTl6HjvgZ0qoBtfs+xjoF\r\nrgpSd0ONSzJAsTRfwjG4Xsv3K0VSKwIYjpF4wi/1AoGBAKjlOmiNWTr34ATVny4Z\r\niS6hCOQWZd/+7nY4zfQEtiKS7Jd1cY9ic9ryXHl4vMiv+prmV33webFK/TxQXHM7\r\nSzspJ42l2f6wuwMjiSAec44EmY1biGE1KEGTMMkWXqgMLjALjVFYFWpYgXQlgW5f\r\n1dx3Ivx8rRH4Ttj2oFvWu/CVAoGBALuDdDyMNAGZAItYJJZ7QDCMigm8on6AOr0E\r\nb+5OXRAFpZdNHyIROo0hMfgwc/cldJTOSKDUeeAQ2aU/nyC8P1gihBflOFUz05iu\r\nLTpIwsoojm2LfbuO/eQy4R2FwfMmIbAZyAUspQs4c91XT/sX9P5xis6zznS2mZ1b\r\n3OoIPmK5AoGAAVUNzBGywcFpR86nmOlXbUJu0aTHEg+J5ktrx+JlVyLOKShVvbKm\r\ne53x8Wu2D/ygtmcGJTXZrQV9VqrJrdUqmGuHVVxg7YjaWwmtrF4Lap5ESBv/WtCK\r\nxjx9KoHY0ZrfXwfrMdPsBAsY6wb2QjpbhI7+8lTV6x+yMEVGTEkpgiU=\r\n-----END RSA PRIVATE KEY-----");
```

