// Helper function to print 64-bit values on platforms that don't support %llu
void printU64(uint64_t value) {
  if (value == 0) {
    CI_LOG("0");
    return;
  }

  char buffer[32];
  int pos = 0;

  while (value > 0) {
    buffer[pos++] = '0' + (value % 10);
    value /= 10;
  }

  // Print in reverse order
  for (int i = pos - 1; i >= 0; i--) {
    char temp[2];
    temp[0] = buffer[i];
    temp[1] = '\0';
    CI_LOG(temp);
  }
}