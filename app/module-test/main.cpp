import TestModule;

int main() {
    TestModule module;
    auto& data = module.GetData();
    data.emplace_back(1);
    return 0;
}
