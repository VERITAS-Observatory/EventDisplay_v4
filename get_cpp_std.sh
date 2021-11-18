if [ ! -f "print_cpp_std.out" ]; then
  CPP_PROGRAM="CiNpbmNsdWRlPGlvc3RyZWFtPgoKaW50IG1haW4oKSB7CglpZiAoX19jcGx1c3BsdXMgPT0gMjAxNzAzTCkgc3RkOjpjb3V0IDw8ICJDKysxN1xuIjsKCWVsc2UgaWYgKF9fY3BsdXNwbHVzID09IDIwMTQwMkwpIHN0ZDo6Y291dCA8PCAiQysrMTRcbiI7CgllbHNlIGlmIChfX2NwbHVzcGx1cyA9PSAyMDExMDNMKSBzdGQ6OmNvdXQgPDwgIkMrKzExXG4iOwoJZWxzZSBpZiAoX19jcGx1c3BsdXMgPT0gMTk5NzExTCkgc3RkOjpjb3V0IDw8ICJDKys5OFxuIjsKCWVsc2Ugc3RkOjpjb3V0IDw8ICJwcmUtc3RhbmRhcmQgQysrXG4iOwp9Cg=="
  echo "$CPP_PROGRAM" | base64 -d | g++ -o print_cpp_std.out -x c++ -
  chmod +x print_cpp_std.out
fi
./print_cpp_std.out | tr '[:upper:]' '[:lower:]'
