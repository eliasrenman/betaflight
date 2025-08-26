/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rx_uid_common.h"

bool generateUIDFromPhrase(const char* phrase, uint8_t* uid) {
    char* phraseCopy = strdup(phrase);
    if (!phraseCopy) {
        return false;
    }
    
    char* token = strtok(phraseCopy, ",");
    int count = 0;
    uint8_t tempUid[6] = {0};
    
    while (token != NULL && count < 6) {
        // Skip leading whitespace
        while (*token == ' ') token++;
        
        // Check if token is a valid number
        bool isValid = true;
        for (int i = 0; token[i] != '\0'; i++) {
            if (token[i] < '0' || token[i] > '9') {
                isValid = false;
                break;
            }
        }
        
        if (isValid) {
            int value = atoi(token);
            if (value >= 0 && value < 256) {
                tempUid[count] = (uint8_t)value;
                count++;
            } else {
                isValid = false;
            }
        }
        
        if (!isValid) {
            free(phraseCopy);
            return false;
        }
        
        token = strtok(NULL, ",");
    }
    
    free(phraseCopy);
    
    // Check if we have 4-6 valid numbers
    if (count < 4 || count > 6) {
        return false;
    }
    
    // Pad to 6 bytes by adding zeros at the beginning
    memset(uid, 0, 6);
    memcpy(&uid[6 - count], tempUid, count);
    
    return true;
}

// MD5 implementation to match ExpressLRS exactly
static void calculateMD5(const char* input, uint8_t* output);

void generateMD5UID(const char* phrase, uint8_t* uid) {
    // Implement the exact ExpressLRS algorithm:
    // 1. Prepend "-DMY_BINDING_PHRASE=\"" to the phrase
    // 2. Hash the entire string with MD5
    // 3. Take the first 6 bytes of the MD5 hash
    
    // Create the full string that ExpressLRS hashes
    char fullPhrase[256];
    int len = strlen(phrase);
    size_t totalLen = len + 24; // "-DMY_BINDING_PHRASE=\"" + phrase + "\""
    
    if (totalLen >= sizeof(fullPhrase)) {
        // Truncate if too long
        totalLen = sizeof(fullPhrase) - 1;
    }
    
    // Manually construct the string to avoid sprintf poisoning
    int pos = 0;
    const char* prefix = "-DMY_BINDING_PHRASE=\"";
    const int prefixLen = strlen(prefix);
    
    memcpy(fullPhrase, prefix, prefixLen);
    pos += prefixLen;
    
    memcpy(fullPhrase + pos, phrase, len);
    pos += len;
    
    fullPhrase[pos++] = '"';
    fullPhrase[pos] = '\0';
    
    // Calculate MD5 hash of the full string
    uint8_t md5Hash[16]; // MD5 produces 16 bytes
    calculateMD5(fullPhrase, md5Hash);
    
    // Take first 6 bytes as UID (ExpressLRS behavior)
    memcpy(uid, md5Hash, 6);
}

static void calculateMD5(const char* input, uint8_t* output) {
    // MD5 constants exactly matching the JavaScript implementation
    const uint32_t k[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };
    
    const uint32_t s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };
    
    uint32_t a = 0x67452301;
    uint32_t b = 0xEFCDAB89;
    uint32_t c = 0x98BADCFE;
    uint32_t d = 0x10325476;
    
    uint32_t len = strlen(input);
    uint32_t msgLen = len * 8;
    
    // Calculate number of 512-bit blocks needed
    uint32_t numBlocks = ((len + 8) >> 6) + 1;
    uint32_t totalLen = numBlocks * 16;
    
    // Allocate memory for the padded message
    uint32_t* message = calloc(totalLen, sizeof(uint32_t));
    if (!message) {
        return;
    }
    
    // Copy input data into message array
    for (uint32_t i = 0; i < len; i++) {
        message[i >> 2] |= ((uint32_t)((uint8_t)input[i])) << (8 * (i % 4));
    }
    
    // Add padding bit
    message[len >> 2] |= 0x80 << (8 * (len % 4));
    
    // Add message length in bits
    message[totalLen - 2] = msgLen & 0xFFFFFFFF;
    message[totalLen - 1] = 0; // For 32-bit systems, upper 32 bits are 0
    
    // Process each 512-bit block
    for (uint32_t i = 0; i < numBlocks; i++) {
        uint32_t aa = a, bb = b, cc = c, dd = d;
        
        for (int j = 0; j < 64; j++) {
            uint32_t f, g;
            
            if (j < 16) {
                f = (bb & cc) | (~bb & dd);
                g = j;
            } else if (j < 32) {
                f = (dd & bb) | (~dd & cc);
                g = (5 * j + 1) % 16;
            } else if (j < 48) {
                f = bb ^ cc ^ dd;
                g = (3 * j + 5) % 16;
            } else {
                f = cc ^ (bb | ~dd);
                g = (7 * j) % 16;
            }
            
            uint32_t temp = dd;
            dd = cc;
            cc = bb;
            bb = bb + ((aa + f + k[j] + message[i * 16 + g]) << s[j] | (aa + f + k[j] + message[i * 16 + g]) >> (32 - s[j]));
            aa = temp;
        }
        
        a += aa;
        b += bb;
        c += cc;
        d += dd;
    }
    
    // Convert final state to bytes (little-endian)
    output[0] = (a >> 0) & 0xFF;
    output[1] = (a >> 8) & 0xFF;
    output[2] = (a >> 16) & 0xFF;
    output[3] = (a >> 24) & 0xFF;
    output[4] = (b >> 0) & 0xFF;
    output[5] = (b >> 8) & 0xFF;
    output[6] = (b >> 16) & 0xFF;
    output[7] = (b >> 24) & 0xFF;
    output[8] = (c >> 0) & 0xFF;
    output[9] = (c >> 8) & 0xFF;
    output[10] = (c >> 16) & 0xFF;
    output[11] = (c >> 24) & 0xFF;
    output[12] = (d >> 0) & 0xFF;
    output[13] = (d >> 8) & 0xFF;
    output[14] = (d >> 16) & 0xFF;
    output[15] = (d >> 24) & 0xFF;
    
    free(message);
}
