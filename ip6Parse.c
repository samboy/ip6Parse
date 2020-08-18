/* Donated to the public domain 2020 Sam Trenholme */

/* ip6Parse: Given a string with a human readable IPv6 IP, and
   an array to put the resulting 16-byte IP, convert the string
   in to an IPv6 IP.

   Input: human, the string with the readable IPv6 IP
   ip6: A 16-byte character array where we will place the processed
        IP.  It is up to the program calling ip6Parse to make sure this
        has 16 bytes to store the resulting IP.
   
   Output code: 1: The IPv6 IP was added
                Less than 0: There was an error adding the IP

   Error codes: -1 to -255 There was a syntax error at this point
	in the string (where point is a negative number; -1 is
	the first character in the string, -2 the second one, etc.)
	-256: The input string is too long

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

int ip6Parse(char *human, unsigned char *ip6) {
	int afterDoubleColonQuads = 1;
	int doubleColonIndex = -1;
	uint16_t thisQuad = 0;	
	int8_t thisHex = 0;
	int outIndex = 0;
	int currentQuad = 0;
	int currentHexDigit = 0;
	char *humanStart = human;

 	char last = 0;
	int index = 0;

	// Pass 1: See if we have a double colon and count the colons
        // after the double colon
	while(*human != 0 && index < 100) {
		if(last == ':' && *human == ':') {
			if(doubleColonIndex != -1) {
				return -(index + 1);
			}
			doubleColonIndex = index;
		}
		if(last != ':' && *human == ':' && doubleColonIndex != -1) {
			afterDoubleColonQuads++;
		}
		last = *human;
		human++;
		index++;
	}
	if(index >= 100) { return -256; /* Error */ }
	
	// Zero out the output ip6, so we do not have to add zeroes with “::”
        for(index = 0 ; index < 16; index++) {
		ip6[index] = 0;
	}

	// Pass 2: Convert the hex numbers in to an IPv6 IP
	human = humanStart;
	index = 0;
	while(*human != 0 && index < 100) {
		thisHex = -1;
		if(*human >= '0' && *human <= '9') {
			thisHex = *human - '0';
		} else if(*human == '_') {
			thisHex = 0;
		} else if(*human >= 'a' && *human <= 'f') {
			thisHex = *human + 10 - 'a';
		} else if(*human >= 'A' && *human <= 'F') {
			thisHex = *human + 10 - 'A';
		} else if(*human != ':' && *human != '-' && *human != ' ') {
			return -(index + 1); // Error
		}

		if(thisHex != -1) {
			thisQuad <<= 4;
			thisQuad += thisHex;
			currentHexDigit++;
			if(currentHexDigit == 4) {
				if(outIndex + 1 >= 16) { return -1; }
				ip6[outIndex + 1] = thisQuad & 0xff;
				ip6[outIndex] = thisQuad >> 8;
				outIndex += 2;
				thisQuad = 0;
				currentHexDigit = 0;
				currentQuad++;
			}
		}
		// CODE HERE: Colon processing
		human++;
		index++;
	}
	if(index >= 100) { return -256; /* Error */ }

	return 1; // Success	
}

#ifdef HAS_MAIN
#include <stdio.h>
int main(int argc, char **argv) {
	char *test;
	uint8_t ip6[16];
	if(argc > 1) {
		test = argv[1];
	} else {
		test = "2001-0db8-1234-5678 0000-0000-0000-0005";
	}
	if(ip6Parse(test, ip6) != 1) {
		puts("ip6Parse returned syntax error");
		return 0;
	}
        printf("%02x%02x-%02x%02x-%02x%02x-%02x%02x " 
		"%02x%02x-%02x%02x-%02x%02x-%02x%02x\n",
               ip6[0], ip6[1], ip6[2], ip6[3],
               ip6[4], ip6[5], ip6[6], ip6[7],
               ip6[8], ip6[9], ip6[10], ip6[11],
               ip6[12], ip6[13], ip6[14], ip6[15]);
	return 0;	
}
#endif // HAS_MAIN
