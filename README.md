This is a public domain implementation of the IPv6 parser in
inet_pton (people who need an inet_pton can use inet_addr to
parse IPv4 addresses, and this to parse IPv6 addresses).

One of the platforms I develop MaraDNS on (a 2003-era version of
MinGW) doesn’t have `inet_pton` to convert strings in to
IPv6 addresses (it has `inet_addr` for IPv4 addresses, but
nothing for IPv6).

That in mind, I am making this public domain C function which
converts a string in to an IPv6 address.

Because MaraDNS allows (sometimes even requires) non-standard IPv6
strings to represent IPv6 addresses, this parser handles both
standard double-colon IPv6 addresses `2001:db8:f00:ba4::32`, 
standard addresses with all eight quads (`2001:db8:f00:ba4:0:0:0:32`), 
and MaraDNS-specific IPv6 forms, such as
`2001-0db8-0f00-0ba4 0000-0000-0000-0032` or 
`20010db8_f00_ba4___0___0___0__32`.
