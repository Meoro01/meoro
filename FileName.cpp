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
        // BOM이 존재하면, 이를 파일에서 제거
        fseek(file, 3, SEEK_SET);  // BOM 이후부터 읽기 시작
    }
    else {
        // BOM이 없으면, 파일 포인터 그대로
        fseek(file, 0, SEEK_SET);
    }
}

// 책 구조체
typedef struct {
    char title[MAX_TITLE];
    char author[MAX_TITLE];
    char genre[MAX_GENRE];
} Book;

// 노래 구조체
typedef struct {
    char title[MAX_TITLE];
    char genre[MAX_GENRE];
    int weight; // 가중치
} Song;

// 힙 구조체
typedef struct {
    Book books[MAX_ELEMENT];
    int book_count;
    Song songs[MAX_ELEMENT];
    int song_count;
} Heap;

// 장르 가중치 구조체
typedef struct {
    char book_genre[MAX_GENRE];
    char song_genre[MAX_GENRE];
    int weight;
} GenreWeight;

GenreWeight genre_weights[MAX_GENRE_WEIGHTS];
int genre_weight_count = 0;

// 힙 초기화
Heap* create_heap() {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    h->book_count = 0;
    h->song_count = 0;
    return h;
}

// 문자열의 앞뒤 공백 제거
void trim(char* str) {
    int start = 0, end = strlen(str) - 1;

    while (start < end && (str[start] == ' ' || str[start] == '\t')) {
        start++;
    }

    while (end > start && (str[end] == ' ' || str[end] == '\t')) {
        end--;
    }

    str[end + 1] = '\0'; // 끝에 NULL 문자 추가
    memmove(str, str + start, end - start + 2); // 문자열 이동
}

void load_data(Heap* heap, const char* filename, const char* type) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("파일을 열 수 없습니다");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // 책 데이터 로딩
        if (strcmp(type, "book") == 0) {
            Book book;
            if (sscanf(line, "%[^-] - %[^-] - %[^\n]", book.title, book.author, book.genre) == 3) {
                printf("읽은 줄: %s\n", line); //
                strtok(book.genre, "/ "); // 장르에서 불필요한 공백이나 슬래시 제거
                trim(book.title); // 제목에 불필요한 공백 제거
                trim(book.author); // 저자에 불필요한 공백 제거
                heap->books[heap->book_count++] = book;
            }
        }
        // 노래 데이터 로딩
        else if (strcmp(type, "song") == 0) {
            Song song;
            if (sscanf(line, "%[^-] - %[^\n]", song.title, song.genre) == 2) {
                strtok(song.genre, "/ "); // 장르에서 불필요한 공백이나 슬래시 제거
                trim(song.title); // 제목에 불필요한 공백 제거
                trim(song.genre); // 장르에 불필요한 공백 제거
                heap->songs[heap->song_count++] = song;
            }
        }
        printf("책 개수: %d\n", heap->book_count);
        printf("노래 개수: %d\n", heap->song_count);
    }

    fclose(file);
}

// 장르 가중치 추가
void add_genre_weight(const char* book_genre, const char* song_genre, int weight) {
    if (genre_weight_count < MAX_GENRE_WEIGHTS) {
        strcpy(genre_weights[genre_weight_count].book_genre, book_genre);
        strcpy(genre_weights[genre_weight_count].song_genre, song_genre);
        genre_weights[genre_weight_count].weight = weight;
        genre_weight_count++;
    }
}

// 책 장르에 따른 음악 가중치 가져오기
int find_genre_weight(const char* book_genre, const char* song_genre) {
    for (int i = 0; i < genre_weight_count; i++) {
        if (strcmp(genre_weights[i].book_genre, book_genre) == 0 &&
            strcmp(genre_weights[i].song_genre, song_genre) == 0) {
            return genre_weights[i].weight;
        }
    }
    return 0; // 기본 가중치
}

// 책 장르 표시
void display_books_by_genre(Heap* heap, const char* genre) {
    printf("'%s' 장르의 책 목록:\n", genre);
    for (int i = 0; i < heap->book_count; i++) {
        if (strcmp(heap->books[i].genre, genre) == 0) {
            printf("- %s (저자: %s)\n", heap->books[i].title, heap->books[i].author);
        }
    }
}

// 음악 추천
void recommend_songs(Heap* heap, const char* book_genre) {
    Song recommendations[MAX_ELEMENT];
    int rec_count = 0, max_weight = 0;

    // 가중치 계산
    for (int i = 0; i < heap->song_count; i++) {
        heap->songs[i].weight = find_genre_weight(book_genre, heap->songs[i].genre);
        if (heap->songs[i].weight > max_weight) {
            max_weight = heap->songs[i].weight;
        }
    }

    // 최고 가중치의 노래 수집
    for (int i = 0; i < heap->song_count; i++) {
        if (heap->songs[i].weight == max_weight) {
            recommendations[rec_count++] = heap->songs[i];
        }
    }

    // 랜덤으로 최대 3곡 추천
    printf("추천 음악 리스트:\n");
    srand(time(NULL));
    for (int i = 0; i < (rec_count > 3 ? 3 : rec_count); i++) {
        int idx = rand() % rec_count;
        printf("- %s (장르: %s, 가중치: %d)\n",
            recommendations[idx].title,
            recommendations[idx].genre,
            recommendations[idx].weight);
    }
}

// 사용자와 상호작용: 책 선택
bool choose_book(Heap* heap, char* selected_title, const char* genre) {
    display_books_by_genre(heap, genre);

    while (true) {
        printf("책 제목을 입력하거나 '다음'을 입력하세요: ");
        scanf(" %[^\n]", selected_title);

        if (strcmp(selected_title, "다음") == 0) {
            for (int i = 0; i < heap->book_count; i++) {
                if (strcmp(heap->books[i].genre, genre) == 0) {
                    printf("추천 책: %s\n", heap->books[i].title);
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
            printf("선택한 책이 장르 목록에 없습니다. 다시 입력하세요.\n");
        }
    }
}

int main() {

    // 로케일 설정
    if (setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
        printf("로케일 설정 실패\n");
        return 1;
    }

    FILE* file = fopen("books.txt", "r");
    if (!file) {
        perror("파일을 열 수 없습니다");
        return 1;
    }

    remove_bom(file);

    // 파일 내용 읽기
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);

    Heap* heap = create_heap();

    // 데이터 로드
    load_data(heap, "books.txt", "book");
    load_data(heap, "songs.txt", "song");

    // 가중치 설정
    add_genre_weight("로맨스", "발라드", 3);
    add_genre_weight("미스테리", "앰비언트", 5);
    add_genre_weight("로맨스", "어쿠스틱", 5);

    // 사용자와 상호작용
    char book_genre[MAX_GENRE];
    printf("책 장르를 입력하세요: ");
    scanf("%s", book_genre);

    char selected_title[MAX_TITLE];
    if (choose_book(heap, selected_title, book_genre)) {
        printf("선택한 책: %s\n", selected_title);
        recommend_songs(heap, book_genre);
    }
    else {
        printf("책 선택이 취소되었습니다.\n");
    }

    free(heap);
    return 0;
}