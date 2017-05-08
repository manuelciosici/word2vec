//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

const long long max_size = 2000;         // max length of strings
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries

int main(int argc, char **argv) {
  FILE *f;
  char st1[max_size];
  char *best_word[N];
  char file_name[max_size], sentence[100][max_size];
  float dist, len, best_distances[N], vector_current_sentence[max_size];
  long long vocabulary_size, num_dimensions, a, b, c, d, num_words_in_sentence, map_sentence_words_to_vocabulary_index[100];
  char ch;
  float *M;
  char *vocab;
  if (argc < 2) {
    printf("Usage: ./distance <FILE>\nwhere FILE contains word projections in the BINARY FORMAT\n");
    return 0;
  }
  strcpy(file_name, argv[1]);
  f = fopen(file_name, "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  fscanf(f, "%lld", &vocabulary_size);
  fscanf(f, "%lld", &num_dimensions);
  vocab = (char *)malloc((long long)vocabulary_size * max_w * sizeof(char));
  for (a = 0; a < N; a++) best_word[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)vocabulary_size * (long long)num_dimensions * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)vocabulary_size * num_dimensions * sizeof(float) / 1048576, vocabulary_size, num_dimensions);
    return -1;
  }
  for (b = 0; b < vocabulary_size; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < num_dimensions; a++) fread(&M[a + b * num_dimensions], sizeof(float), 1, f);
//    since we're using the cosine distance, we can go ahead and already divide all by square root of sun of squares
//    (covers next 4 lines)
    len = 0;
    for (a = 0; a < num_dimensions; a++) len += M[a + b * num_dimensions] * M[a + b * num_dimensions];
    len = sqrt(len);
    for (a = 0; a < num_dimensions; a++) M[a + b * num_dimensions] /= len;
  }
  fclose(f);
  while (1) {
    for (a = 0; a < N; a++) best_distances[a] = 0;
    for (a = 0; a < N; a++) best_word[a][0] = 0;
    printf("Enter word or sentence (EXIT to break): ");
    a = 0;
//    read word or sentence followed by the new line character
    while (1) {
      st1[a] = fgetc(stdin);
      if ((st1[a] == '\n') || (a >= max_size - 1)) {
        st1[a] = 0;
        break;
      }
      a++;
    }
    if (!strcmp(st1, "EXIT")) break;
    num_words_in_sentence = 0;
    b = 0;
    c = 0;
    while (1) {
      sentence[num_words_in_sentence][b] = st1[c];
      b++;
      c++;
      sentence[num_words_in_sentence][b] = 0;
      if (st1[c] == 0) break;
      if (st1[c] == ' ') {
        num_words_in_sentence++;
        b = 0;
        c++;
      }
    }
    num_words_in_sentence++;
    for (a = 0; a < num_words_in_sentence; a++) {
      for (b = 0; b < vocabulary_size; b++) if (!strcmp(&vocab[b * max_w], sentence[a])) break;
//      if b is vocabulary_size then the word is not in the vocabulary
      if (b == vocabulary_size) b = -1;
      map_sentence_words_to_vocabulary_index[a] = b;
      printf("\nWord: %s  Position in vocabulary: %lld\n", sentence[a], map_sentence_words_to_vocabulary_index[a]);
//    stop evaluating when encountering a word not in the vocabulary
      if (b == -1) {
        printf("Out of dictionary word!\n");
        break;
      }
    }
//    last word analysed was not in the vocabulary, continue to new loop
    if (b == -1) continue;
    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
    for (a = 0; a < num_dimensions; a++) vector_current_sentence[a] = 0;
    for (b = 0; b < num_words_in_sentence; b++) {
      if (map_sentence_words_to_vocabulary_index[b] == -1) continue;
      for (a = 0; a < num_dimensions; a++) vector_current_sentence[a] += M[a + map_sentence_words_to_vocabulary_index[b] * num_dimensions];
    }
//    same as above, we can divide by square root of sum of squares
    len = 0;
    for (a = 0; a < num_dimensions; a++) len += vector_current_sentence[a] * vector_current_sentence[a];
    len = sqrt(len);
    for (a = 0; a < num_dimensions; a++) vector_current_sentence[a] /= len;

    for (a = 0; a < N; a++) best_distances[a] = -1;
    for (a = 0; a < N; a++) best_word[a][0] = 0;
//    iterate over all words in the vocabulary
    for (c = 0; c < vocabulary_size; c++) {
      a = 0;
//      if the current word is present in the sentence, then ignore the current word
      for (b = 0; b < num_words_in_sentence; b++) if (map_sentence_words_to_vocabulary_index[b] == c) a = 1;
      if (a == 1) continue;
//      calculate distance to current word
      dist = 0;
      for (a = 0; a < num_dimensions; a++) dist += vector_current_sentence[a] * M[a + c * num_dimensions];
//      if the current word is closer than some other word in best
      for (a = 0; a < N; a++) {
        if (dist > best_distances[a]) {
          for (d = N - 1; d > a; d--) {
            best_distances[d] = best_distances[d - 1];
            strcpy(best_word[d], best_word[d - 1]);
          }
          best_distances[a] = dist;
          strcpy(best_word[a], &vocab[c * max_w]);
          break;
        }
      }
    }
    for (a = 0; a < N; a++) printf("%50s\t\t%f\n", best_word[a], best_distances[a]);
  }
  return 0;
}
