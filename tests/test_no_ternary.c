// Sanity check: ensure non-ternary code still works
int normal_add(int a, int b) {
    return a + b;
}

int main() {
    return normal_add(1, 2);
}