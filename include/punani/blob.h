#ifndef _PUNANI_BLOB_H
#define _PUNANI_BLOB_H

void blob_free(uint8_t *blob, size_t sz);
uint8_t *blob_from_file(const char *fn, size_t *sz);

#endif /* _PUNANI_BLOB_H */