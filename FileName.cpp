#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#define MAX_ELEMENT 200
#define MAX_TITLE 100
#define MAX_GENRE 50
#define MAX_GENRE_WEIGHTS 20

#define BOM 0xEFBBBF

void remove_bom(FILE* file) {
    unsigned char bom[3];
    fread(bom, 1, 3, file);

    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        // BOM�� �����ϸ�, �̸� ���Ͽ��� ����
        fseek(file, 3, SEEK_SET);  // BOM ���ĺ��� �б� ����
    }
    else {
        // BOM�� ������, ���� ������ �״��
        fseek(file, 0, SEEK_SET);
    }
}

// å ����ü
typedef struct {
    char title[MAX_TITLE];
    char author[MAX_TITLE];
    char genre[MAX_GENRE];
} Book;

// �뷡 ����ü
typedef struct {
    char title[MAX_TITLE];
    char genre[MAX_GENRE];
    int weight; // ����ġ
} Song;

// �� ����ü
typedef struct {
    Book books[MAX_ELEMENT];
    int book_count;
    Song songs[MAX_ELEMENT];
    int song_count;
} Heap;

// �帣 ����ġ ����ü
typedef struct {
    char book_genre[MAX_GENRE];
    char song_genre[MAX_GENRE];
    int weight;
} GenreWeight;

GenreWeight genre_weights[MAX_GENRE_WEIGHTS];
int genre_weight_count = 0;

// �� �ʱ�ȭ
Heap* create_heap() {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    h->book_count = 0;
    h->song_count = 0;
    return h;
}

// ���ڿ��� �յ� ���� ����
void trim(char* str) {
    int start = 0, end = strlen(str) - 1;

    while (start < end && (str[start] == ' ' || str[start] == '\t')) {
        start++;
    }

    while (end > start && (str[end] == ' ' || str[end] == '\t')) {
        end--;
    }

    str[end + 1] = '\0'; // ���� NULL ���� �߰�
    memmove(str, str + start, end - start + 2); // ���ڿ� �̵�
}

void load_data(Heap* heap, const char* filename, const char* type) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("������ �� �� �����ϴ�");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // å ������ �ε�
        if (strcmp(type, "book") == 0) {
            Book book;
            if (sscanf(line, "%[^-] - %[^-] - %[^\n]", book.title, book.author, book.genre) == 3) {
                printf("���� ��: %s\n", line); //
                strtok(book.genre, "/ "); // �帣���� ���ʿ��� �����̳� ������ ����
                trim(book.title); // ���� ���ʿ��� ���� ����
                trim(book.author); // ���ڿ� ���ʿ��� ���� ����
                heap->books[heap->book_count++] = book;
            }
        }
        // �뷡 ������ �ε�
        else if (strcmp(type, "song") == 0) {
            Song song;
            if (sscanf(line, "%[^-] - %[^\n]", song.title, song.genre) == 2) {
                strtok(song.genre, "/ "); // �帣���� ���ʿ��� �����̳� ������ ����
                trim(song.title); // ���� ���ʿ��� ���� ����
                trim(song.genre); // �帣�� ���ʿ��� ���� ����
                heap->songs[heap->song_count++] = song;
            }
        }
        printf("å ����: %d\n", heap->book_count);
        printf("�뷡 ����: %d\n", heap->song_count);
    }

    fclose(file);
}

// �帣 ����ġ �߰�
void add_genre_weight(const char* book_genre, const char* song_genre, int weight) {
    if (genre_weight_count < MAX_GENRE_WEIGHTS) {
        strcpy(genre_weights[genre_weight_count].book_genre, book_genre);
        strcpy(genre_weights[genre_weight_count].song_genre, song_genre);
        genre_weights[genre_weight_count].weight = weight;
        genre_weight_count++;
    }
}

// å �帣�� ���� ���� ����ġ ��������
int find_genre_weight(const char* book_genre, const char* song_genre) {
    for (int i = 0; i < genre_weight_count; i++) {
        if (strcmp(genre_weights[i].book_genre, book_genre) == 0 &&
            strcmp(genre_weights[i].song_genre, song_genre) == 0) {
            return genre_weights[i].weight;
        }
    }
    return 0; // �⺻ ����ġ
}

// å �帣 ǥ��
void display_books_by_genre(Heap* heap, const char* genre) {
    printf("'%s' �帣�� å ���:\n", genre);
    for (int i = 0; i < heap->book_count; i++) {
        if (strcmp(heap->books[i].genre, genre) == 0) {
            printf("- %s (����: %s)\n", heap->books[i].title, heap->books[i].author);
        }
    }
}

// ���� ��õ
void recommend_songs(Heap* heap, const char* book_genre) {
    Song recommendations[MAX_ELEMENT];
    int rec_count = 0, max_weight = 0;

    // ����ġ ���
    for (int i = 0; i < heap->song_count; i++) {
        heap->songs[i].weight = find_genre_weight(book_genre, heap->songs[i].genre);
        if (heap->songs[i].weight > max_weight) {
            max_weight = heap->songs[i].weight;
        }
    }

    // �ְ� ����ġ�� �뷡 ����
    for (int i = 0; i < heap->song_count; i++) {
        if (heap->songs[i].weight == max_weight) {
            recommendations[rec_count++] = heap->songs[i];
        }
    }

    // �������� �ִ� 3�� ��õ
    printf("��õ ���� ����Ʈ:\n");
    srand(time(NULL));
    for (int i = 0; i < (rec_count > 3 ? 3 : rec_count); i++) {
        int idx = rand() % rec_count;
        printf("- %s (�帣: %s, ����ġ: %d)\n",
            recommendations[idx].title,
            recommendations[idx].genre,
            recommendations[idx].weight);
    }
}

// ����ڿ� ��ȣ�ۿ�: å ����
bool choose_book(Heap* heap, char* selected_title, const char* genre) {
    display_books_by_genre(heap, genre);

    while (true) {
        printf("å ������ �Է��ϰų� '����'�� �Է��ϼ���: ");
        scanf(" %[^\n]", selected_title);

        if (strcmp(selected_title, "����") == 0) {
            for (int i = 0; i < heap->book_count; i++) {
                if (strcmp(heap->books[i].genre, genre) == 0) {
                    printf("��õ å: %s\n", heap->books[i].title);
                    break;
                }
            }
        }
        else {
            for (int i = 0; i < heap->book_count; i++) {
                if (strcmp(heap->books[i].genre, genre) == 0 &&
                    strcmp(heap->books[i].title, selected_title) == 0) {
                    return true;
                }
            }
            printf("������ å�� �帣 ��Ͽ� �����ϴ�. �ٽ� �Է��ϼ���.\n");
        }
    }
}

int main() {

    // ������ ����
    if (setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
        printf("������ ���� ����\n");
        return 1;
    }

    FILE* file = fopen("books.txt", "r");
    if (!file) {
        perror("������ �� �� �����ϴ�");
        return 1;
    }

    remove_bom(file);

    // ���� ���� �б�
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);

    Heap* heap = create_heap();

    // ������ �ε�
    load_data(heap, "books.txt", "book");
    load_data(heap, "songs.txt", "song");

    // ����ġ ����
    add_genre_weight("�θǽ�", "�߶��", 3);
    add_genre_weight("�̽��׸�", "�ں��Ʈ", 5);
    add_genre_weight("�θǽ�", "����ƽ", 5);

    // ����ڿ� ��ȣ�ۿ�
    char book_genre[MAX_GENRE];
    printf("å �帣�� �Է��ϼ���: ");
    scanf("%s", book_genre);

    char selected_title[MAX_TITLE];
    if (choose_book(heap, selected_title, book_genre)) {
        printf("������ å: %s\n", selected_title);
        recommend_songs(heap, book_genre);
    }
    else {
        printf("å ������ ��ҵǾ����ϴ�.\n");
    }

    free(heap);
    return 0;
}