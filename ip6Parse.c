/* Donated to the public domain 2020 Sam Trenholme */

/* ip6Parse: Given a string with a human readable IPv6 IP, and
   an array to put the resulting 16-byte IP, convert the string
   in to an IPv6 IP.

   Input: human, the string with the readable IPv6 IP

   len, the length of the "human" string (make -1 to have human be
	null-terminated)

   ip6: A 16-byte character array where we will place the processed
        IP.  It is up to the program calling ip6Parse to make sure this
        has 16 bytes to store the resulting IP.
   
   Output code: 1: The IPv6 IP was added
                Less than 0: There was an error adding the IP

   Error codes: -1 to -255 There was a syntax error at this point
	in the string (where point is a negative number; -1 is
	the first character in the string, -2 the second one, etc.)
	-256: The input string is too long
	-257: There are too many hex digits or colons in the input string
	-258: Unexpected error occurred parsing "::" sequence
	-259: Too many colons
	-260: No colons and we do not have 32 hex digits in string
	-261: More than four hex digits in a row and ":" present in string
	-262: len parameter out of bounds
	-263: Trailing colon in name

   Rules: Single colon separates 16-bit (4 hex digit) numbers
          Double colon (only one allowed or error) means 
          “This is a string of zeroes”; it allows servers with lots
          of zeroes in their IPv6 IP to be represented more compactly
          _ is treates like a 0 (Deadwood 3.3.02/3.4 IP6 address format)
          “-” and space are ignored.

   Acceptable input strings:

   ::1
   2001:0db8:1234:5678::5
   2001:db8:1234:5678::5
   2001-0db8-1234-5678 0000-0000-0000-0005
   2001_db812345678__00__00__00__05
   2001:DB8:1234:5678::5

   A single 16-bit number is called a “Quad” in the source code

 */

#include <stdint.h>

int ip6Parse(char *human, int len, unsigned char *ip6) {
	int afterDoubleColonQuads = 1;
	int doubleColonIndex = -1;
	uint16_t thisQuad = 0;	
	int8_t thisHex = 0;
	int outIndex = 0;
	int currentQuad = 0;
	int currentHexDigit = 0;
	char *humanStart = human;
	int hexCount = 0;
	int colonCount = 0;

 	char last = 0;
	int index = 0;

	if(len > 75 || (len < 2 && len != -1)) { return -262; }
	// Pass 1: See if we have a double colon and count the colons
        // after the double colon.  Also: Count colons
	while((len == -1 && *human != 0 && index < 100) || index < len) {
		if(last == ':' && *human == ':') {
			if(doubleColonIndex != -1) {
				return -(index + 1);
			}
			doubleColonIndex = index;
		}
		if(last != ':' && *human == ':' && doubleColonIndex != -1) {
			afterDoubleColonQuads++;
		}
		if(*human == ':') { colonCount++; }
		last = *human;
		human++;
		index++;
	}
	// Trailing colon error (2001:db8::1:2000:) needs an explicit check
	if(last == ':' && doubleColonIndex != index - 1) { return -263; } 
	if(index >= 100) { return -256; /* Error */ }
	
	// Zero out the output ip6, so we do not have to add zeroes with “::”
        for(index = 0 ; index < 16; index++) {
		ip6[index] = 0;
	}

	// Pass 2: Convert the hex numbers in to an IPv6 IP
	human = humanStart;
	index = 0;
	while((len == -1 && *human != 0 && index < 100) || index < len) {
		thisHex = -1;
		if(*human >= '0' && *human <= '9') {
			thisHex = *human - '0';
		} else if(*human == '_' && colonCount == 0) {
			thisHex = 0;
		} else if(*human >= 'a' && *human <= 'f') {
			thisHex = *human + 10 - 'a';
		} else if(*human >= 'A' && *human <= 'F') {
			thisHex = *human + 10 - 'A';
		} else if(*human != ':' && *human != '-' && *human != ' ') {
			return -(index + 1); // Error
		}

		// If hexadecimal digit [0-9a-fA-F] seen, add it to Quad
		if(thisHex != -1) {
			thisQuad <<= 4;
			thisQuad += thisHex;
			currentHexDigit++;
			// Maximum 4 hex digits between colons
			if(currentHexDigit == 5 && colonCount != 0) {
				return -261;
			}
			hexCount++;
		}

		// Single colon processing: End current Quad
		if(*human == ':' && index != doubleColonIndex &&
				currentHexDigit != 0 && currentHexDigit != 4) {
			currentHexDigit = 8;
		}
		if(*human == ':' && index != doubleColonIndex &&
				currentHexDigit == 4) {
			currentHexDigit = 0;
		}

		if(*human == ':' && index == doubleColonIndex) {
			if(thisQuad != 0 || currentHexDigit != 0) { 
				return -258; 
			}
			if(currentQuad + afterDoubleColonQuads >= 8) {
				return -259; // Too many colons
			}
			currentQuad = 8 - afterDoubleColonQuads;
			if(currentQuad < 0) { return -259; }
			outIndex = currentQuad * 2;
		}

		human++;
		index++;
		// Convert a series of up to four hex digits in to raw IPv6
		if((currentHexDigit == 4 && thisHex != -1) || 
				currentHexDigit == 8 || *human == 0
				|| (len != -1 && index >= len)) {
			if(outIndex + 1 >= 16) { return -257; }
			ip6[outIndex + 1] = thisQuad & 0xff;
			ip6[outIndex] = thisQuad >> 8;
			outIndex += 2;
			thisQuad = 0;
			if(currentHexDigit == 8) {
				currentHexDigit = 0;
			}
			currentQuad++;
		}

	}
	if(index >= 100) { return -256; /* Error */ }
	if(colonCount == 0 && hexCount != 32) { return -260; }
	if(colonCount != 0 && currentQuad != 8) { return -262; }

	return 1; // Success	
}

#ifdef HAS_MAIN
#include <stdio.h>
int run_test(char *test, int len, uint8_t *ip6) {
	int result;
	result = ip6Parse(test, len, ip6);
	if(result != 1) {
		printf("ip6Parse returned error code %d\n",result);
		return -1;
	}
        printf("%02x%02x-%02x%02x-%02x%02x-%02x%02x " 
		"%02x%02x-%02x%02x-%02x%02x-%02x%02x\n",
               ip6[0], ip6[1], ip6[2], ip6[3],
               ip6[4], ip6[5], ip6[6], ip6[7],
               ip6[8], ip6[9], ip6[10], ip6[11],
               ip6[12], ip6[13], ip6[14], ip6[15]);
	return 0;	
}

int main(int argc, char **argv) {
	char *test;
	uint8_t ip6[16];
	int a;
	if(argc == 2) {
		test = argv[1];
	} else if(argc == 1) {
		test = "2001-0db8-1234-5678 0000-0000-0000-0005";
	} else {
		puts("Usage: ip6Parse-test -or- ip6Parse-test {IPv6}");
		return 1;
	}
	run_test(test, -1, ip6);
	test = "2001:0db8::1234:5678:8abc:deff";
	if(argc == 1) {
		for(a=11;a<30;a++) {
			run_test(test,a,ip6);
		}
	}		
	return 0;
}
#endif // HAS_MAIN
