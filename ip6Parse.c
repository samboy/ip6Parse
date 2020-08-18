/* Donated to the public domain 2020 Sam Trenholme */

/* ip6Parse: Given a string with a human readable IPv6 IP, and
   an array to put the resulting 16-byte IP, convert the string
   in to an IPv6 IP.

   Input: human, the string with the readable IPv6 IP
   ip6: A 16-byte character array where we will place the processed
        IP.  It is up to the program calling ip6Parse to make sure this
        has 16 bytes to store the resulting IP.
   
   Output code: 1: The IPv6 IP was added
                -1: There was an error adding the IP

 */


