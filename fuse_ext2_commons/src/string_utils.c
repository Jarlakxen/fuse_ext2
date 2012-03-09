/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void string_utils_do(char *text, void (*closure)(char*));
static void string_utils_lower_element(char* ch);
static void string_utils_upper_element(char* ch);

/**
 * @NAME: string_utils_repeat
 * @DESC: Crea un string de longitud 'count' con el mismo caracter.
 *
 * Ejemplo:
 * string_utils_repeat('a', 5) = "aaaaa"
 *
 */
char *string_utils_repeat(char character, int count) {
	char *text = calloc(count + 1, 1);
	int i = 0;
	for (i = 0; i < count; ++i) {
		text[i] = character;
	}
	return text;
}

/**
 * @NAME: string_utils_append
 * @DESC: Agrega al primer string el segundo
 *
 * Ejemplo:
 * char *unaPalabra = "HOLA ";
 * char *otraPalabra = "PEPE";
 *
 * string_utils_append(&unaPalabra, otraPalabra);
 *
 * => unaPalabra = "HOLA PEPE"
 */
void string_utils_append(char** original, char* string_to_add) {
	*original = realloc(*original, strlen(*original) + strlen(string_to_add) + 1);
	strcat(*original, string_to_add);
}

/**
 * @NAME: string_utils_to_upper
 * @DESC: Pone en mayuscula todos los caracteres de un string
 */
void string_utils_to_upper(char *text) {
	string_utils_do(text, &string_utils_upper_element);
}

/**
 * @NAME: string_utils_to_lower
 * @DESC: Pone en minuscula todos los caracteres de un string
 */
void string_utils_to_lower(char *text) {
	string_utils_do(text, &string_utils_lower_element);
}

/**
 * @NAME: string_utils_capitalized
 * @DESC: Capitaliza un string
 */
void string_utils_capitalized(char *text) {
	if (!string_utils_is_empty(text)) {
		string_utils_upper_element(text);
		if (strlen(text) >= 2){
			string_utils_to_lower(&text[1]);
		}
	}
}

/**
 * @NAME: string_utils_trim
 * @DESC: Remueve todos los caracteres
 * vacios de la derecha y la izquierda
 */
void string_utils_trim(char** text) {
	string_utils_trim_left(text);
	string_utils_trim_right(text);
}

/**
 * @NAME: string_trim_left
 * @DESC: Remueve todos los caracteres
 * vacios de la izquierda
 */
void string_utils_trim_left(char** text) {
	char *string_without_blank = *text;

	while (isblank(*string_without_blank)) {
		string_without_blank++;
	}

	char *new_string = strdup(string_without_blank);

	free(*text);
	*text = new_string;
}

/**
 * @NAME: string_utils_trim_right
 * @DESC: Remueve todos los caracteres
 * vacios de la derecha
 */
void string_utils_trim_right(char** text) {
	char *string_without_blank = *text;
	int i = strlen(*text) - 1;
	while (i >= 0 && isspace(string_without_blank[i])){
		string_without_blank[i] = '\0';
		i--;
	}
	*text = realloc(*text, strlen(string_without_blank) + 1);
}

/**
 * @NAME: string_utils_is_empty
 * @DESC: Retorna si un string es ""
 */
bool string_utils_is_empty(char *text) {
	return strlen(text) == 0;
}

/**
 * @NAME: string_starts_with
 * @DESC: Retorna si un string comienza con el
 * string pasado por parametro
 */
bool string_utils_starts_with(char *text, char *begin){
	return strncmp(text, begin, strlen(begin)) == 0;
}

/**
 * @NAME: string_utils_equals_ignore_case
 * @DESC: Retorna si dos strings son iguales
 * ignorando las mayusculas y minusculas
 */
bool string_utils_equals_ignore_case(char *actual, char *expected) {
	return strcasecmp(actual, expected) == 0;
}


/**
 * @NAME: string_utils_split
 * @DESC: Separa un string dado un separador
 */
char **string_utils_split(char *text, char *regex) {
	char **substrings = NULL;
	int size = 0;

	char *text_to_iterate = strdup(text);
	char *token = NULL, *next = NULL;
	for (token = strtok_r(text_to_iterate, regex, &next) ;
		 token ;
		 token = strtok_r(NULL, regex, &next)) {

		size++;
		substrings = realloc(substrings, sizeof(char*) * size);
		substrings[size - 1] = strdup(token);
	}

	size++;
	substrings = realloc(substrings, sizeof(char*) * size);
	substrings[size - 1] = NULL;

	free(text_to_iterate);
	return substrings;
}

/**
 * @NAME: string_utils_iterate_lines
 * @DESC: Itera un array de strings aplicando
 * el closure a cada string
 */
void string_utils_iterate_lines(char** strings, void (*closure)(char*)) {
	while (*strings != NULL) {
		closure(*strings);
		strings++;
	}
}


/** PRIVATE FUNCTIONS **/

static void string_utils_upper_element(char* ch) {
	*ch = toupper(*ch);
}

static void string_utils_lower_element(char* ch) {
	*ch = tolower(*ch);
}

static void string_utils_do(char *text, void (*closure)(char* c)){
	int i = 0;
	while (text[i] != '\0') {
		closure(&text[i]);
		i++;
	}
}
