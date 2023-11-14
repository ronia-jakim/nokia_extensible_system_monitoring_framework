#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* format(int parametr, char* parametrName, int id_węzła, int id_pluginu, int time) {
    int dlugosc_wyniku = snprintf(NULL, 0, "{Headers:{%d,%d,Opis_DATA{%s: int},LIST[{%d,%d}]}}}", id_węzła, id_pluginu, parametrName, time, parametr);

    char* wynik = malloc((dlugosc_wyniku + 1) * sizeof(char));  // +1 na znak końca ciągu '\0'

    if (wynik == NULL) {
        perror("Błąd alokacji pamięci");
        exit(EXIT_FAILURE);
    }

    snprintf(wynik, dlugosc_wyniku + 1, "{Headers:{%d,%d,Opis_DATA{%s: int},LIST[{%d,%d}]}}}", id_węzła, id_pluginu, parametrName, time, parametr);

    return wynik;
}

int main() {
    // Odczytanie wartości z potoku
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Konwersja odczytanej wartości na liczbę
        int memory_usage = atoi(buffer);

        // Odczytanie czasu wykonania z potoku
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            int execution_time = atoi(buffer);

            // Wywołanie funkcji format
            char* wynik = format(memory_usage, "Ram", 1, 1, execution_time);

            // Wyświetlanie wyniku
            printf("Wynik: %s\n", wynik);

            // Zwolnienie pamięci
            free(wynik);
        } else {
            printf("Błąd odczytu czasu wykonania z potoku\n");
        }
    } else {
        printf("Błąd odczytu z potoku\n");
    }

    return 0;
}
