int put_character(int no, char c) {
    __asm__ __volatile__ (
    "svc #0"
    );
}

int main() {
    while (1) {
        put_character(0, 'a');
    }
    return 0;
}

