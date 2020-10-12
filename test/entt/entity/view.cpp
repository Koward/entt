#include <utility>
#include <type_traits>
#include <gtest/gtest.h>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>

struct empty_type {};

TEST(SingleComponentView, Functionalities) {
    entt::registry registry;
    auto view = registry.view<char>();
    auto cview = std::as_const(registry).view<const char>();

    const auto e0 = registry.create();
    const auto e1 = registry.create();

    ASSERT_TRUE(view.empty());

    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    ASSERT_NO_THROW(view.begin()++);
    ASSERT_NO_THROW(++cview.begin());
    ASSERT_NO_THROW([](auto it) { return it++; }(view.rbegin()));
    ASSERT_NO_THROW([](auto it) { return ++it; }(cview.rbegin()));

    ASSERT_NE(view.begin(), view.end());
    ASSERT_NE(cview.begin(), cview.end());
    ASSERT_NE(view.rbegin(), view.rend());
    ASSERT_NE(cview.rbegin(), cview.rend());
    ASSERT_EQ(view.size(), 1u);
    ASSERT_FALSE(view.empty());

    registry.emplace<char>(e0);

    ASSERT_EQ(view.size(), 2u);

    view.get<char>(e0) = '1';
    view.get(e1) = '2';

    for(auto entity: view) {
        ASSERT_TRUE(cview.get<const char>(entity) == '1' || cview.get(entity) == '2');
    }

    ASSERT_EQ(*(view.data() + 0), e1);
    ASSERT_EQ(*(view.data() + 1), e0);

    ASSERT_EQ(*(view.raw() + 0), '2');
    ASSERT_EQ(*(cview.raw() + 1), '1');

    registry.remove<char>(e0);
    registry.remove<char>(e1);

    ASSERT_EQ(view.begin(), view.end());
    ASSERT_EQ(view.rbegin(), view.rend());
    ASSERT_TRUE(view.empty());
}

TEST(SingleComponentView, ElementAccess) {
    entt::registry registry;
    auto view = registry.view<int>();
    auto cview = std::as_const(registry).view<const int>();

    const auto e0 = registry.create();
    registry.emplace<int>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);

    for(auto i = 0u; i < view.size(); ++i) {
        ASSERT_EQ(view[i], i ? e0 : e1);
        ASSERT_EQ(cview[i], i ? e0 : e1);
    }
}

TEST(SingleComponentView, Contains) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<int>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);

    registry.destroy(e0);

    auto view = registry.view<int>();

    ASSERT_FALSE(view.contains(e0));
    ASSERT_TRUE(view.contains(e1));
}

TEST(SingleComponentView, Empty) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<char>(e0);
    registry.emplace<double>(e0);

    const auto e1 = registry.create();
    registry.emplace<char>(e1);

    auto view = registry.view<int>();

    ASSERT_EQ(view.size(), 0u);
    ASSERT_EQ(view.begin(), view.end());
    ASSERT_EQ(view.rbegin(), view.rend());
}

TEST(SingleComponentView, Each) {
    entt::registry registry;

    registry.emplace<int>(registry.create());
    registry.emplace<int>(registry.create());

    auto view = registry.view<int>();
    auto cview = std::as_const(registry).view<const int>();
    std::size_t cnt = 0;

    view.each([&cnt](auto, int &) { ++cnt; });
    view.each([&cnt](int &) { ++cnt; });

    for(auto &&[entt, iv]: view.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        ++cnt;
    }

    ASSERT_EQ(cnt, std::size_t{6});

    cview.each([&cnt](auto, const int &) { --cnt; });
    cview.each([&cnt](const int &) { --cnt; });

    for(auto &&[entt, iv]: cview.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), const int &>);
        --cnt;
    }

    ASSERT_EQ(cnt, std::size_t{0});
}

TEST(SingleComponentView, ConstNonConstAndAllInBetween) {
    entt::registry registry;
    auto view = registry.view<int>();
    auto cview = std::as_const(registry).view<const int>();

    ASSERT_EQ(view.size(), decltype(view.size()){0});
    ASSERT_EQ(cview.size(), decltype(cview.size()){0});

    registry.emplace<int>(registry.create(), 0);

    ASSERT_EQ(view.size(), decltype(view.size()){1});
    ASSERT_EQ(cview.size(), decltype(cview.size()){1});

    static_assert(std::is_same_v<typename decltype(view)::raw_type, int>);
    static_assert(std::is_same_v<typename decltype(cview)::raw_type, const int>);

    static_assert(std::is_same_v<decltype(view.get({})), int &>);
    static_assert(std::is_same_v<decltype(view.raw()), int *>);
    static_assert(std::is_same_v<decltype(cview.get({})), const int &>);
    static_assert(std::is_same_v<decltype(cview.raw()), const int *>);

    view.each([](auto &&i) {
        static_assert(std::is_same_v<decltype(i), int &>);
    });

    cview.each([](auto &&i) {
        static_assert(std::is_same_v<decltype(i), const int &>);
    });

    for(auto &&[entt, iv]: view.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
    }

    for(auto &&[entt, iv]: cview.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), const int &>);
    }
}

TEST(SingleComponentView, Find) {
    entt::registry registry;
    auto view = registry.view<int>();

    const auto e0 = registry.create();
    registry.emplace<int>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);

    const auto e2 = registry.create();
    registry.emplace<int>(e2);

    const auto e3 = registry.create();
    registry.emplace<int>(e3);

    registry.remove<int>(e1);

    ASSERT_NE(view.find(e0), view.end());
    ASSERT_EQ(view.find(e1), view.end());
    ASSERT_NE(view.find(e2), view.end());
    ASSERT_NE(view.find(e3), view.end());

    auto it = view.find(e2);

    ASSERT_EQ(*it, e2);
    ASSERT_EQ(*(++it), e3);
    ASSERT_EQ(*(++it), e0);
    ASSERT_EQ(++it, view.end());
    ASSERT_EQ(++view.find(e0), view.end());

    const auto e4 = registry.create();
    registry.destroy(e4);
    const auto e5 = registry.create();
    registry.emplace<int>(e5);

    ASSERT_NE(view.find(e5), view.end());
    ASSERT_EQ(view.find(e4), view.end());
}

TEST(SingleComponentView, EmptyTypes) {
    entt::registry registry;

    auto eview = registry.view<empty_type>();
    auto iview = registry.view<int>();

    auto create = [&](auto... component) {
        const auto entt = registry.create();
        (registry.emplace<decltype(component)>(entt, component), ...);
        return entt;
    };

    const auto entity = create(0, empty_type{});
    create('c');

    eview.each([entity](const auto entt) {
        ASSERT_EQ(entity, entt);
    });

    eview.each([check = true]() mutable {
        ASSERT_TRUE(check);
        check = false;
    });

    for(auto &&[entt]: eview.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        ASSERT_EQ(entity, entt);
    }

    iview.each([entity](const auto entt, int) {
        ASSERT_EQ(entity, entt);
    });

    iview.each([check = true](int) mutable {
        ASSERT_TRUE(check);
        check = false;
    });

    for(auto &&[entt, iv]: iview.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        ASSERT_EQ(entity, entt);
    }
}

TEST(SingleComponentView, FrontBack) {
    entt::registry registry;
    auto view = registry.view<const int>();

    ASSERT_EQ(view.front(), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(view.back(), static_cast<entt::entity>(entt::null));

    const auto e0 = registry.create();
    registry.emplace<int>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);

    ASSERT_EQ(view.front(), e1);
    ASSERT_EQ(view.back(), e0);
}

TEST(MultiComponentView, Functionalities) {
    entt::registry registry;
    auto view = registry.view<int, char>();
    auto cview = std::as_const(registry).view<const int, const char>();

    const auto e0 = registry.create();
    registry.emplace<char>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    ASSERT_EQ(*view.begin(), e1);
    ASSERT_EQ(*view.rbegin(), e1);
    ASSERT_EQ(++view.begin(), (view.end()));
    ASSERT_EQ(++view.rbegin(), (view.rend()));

    ASSERT_NO_THROW((view.begin()++));
    ASSERT_NO_THROW((++cview.begin()));
    ASSERT_NO_THROW(view.rbegin()++);
    ASSERT_NO_THROW(++cview.rbegin());

    ASSERT_NE(view.begin(), view.end());
    ASSERT_NE(cview.begin(), cview.end());
    ASSERT_NE(view.rbegin(), view.rend());
    ASSERT_NE(cview.rbegin(), cview.rend());
    ASSERT_EQ(view.size_hint(), decltype(view.size_hint()){1});

    registry.get<char>(e0) = '1';
    registry.get<char>(e1) = '2';
    registry.get<int>(e1) = 42;

    for(auto entity: view) {
        ASSERT_EQ(std::get<0>(cview.get<const int, const char>(entity)), 42);
        ASSERT_EQ(std::get<1>(view.get<int, char>(entity)), '2');
        ASSERT_EQ(cview.get<const char>(entity), '2');
    }
}

TEST(MultiComponentView, Iterator) {
    entt::registry registry;
    const auto entity = registry.create();
    registry.emplace<int>(entity);
    registry.emplace<char>(entity);

    const auto view = registry.view<int, char>();
    using iterator = typename decltype(view)::iterator;

    iterator end{view.begin()};
    iterator begin{};
    begin = view.end();
    std::swap(begin, end);

    ASSERT_EQ(begin, view.begin());
    ASSERT_EQ(end, view.end());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, view.begin());
    ASSERT_EQ(begin--, view.end());

    ASSERT_EQ(++begin, view.end());
    ASSERT_EQ(--begin, view.begin());

    ASSERT_EQ(*begin, entity);
    ASSERT_EQ(*begin.operator->(), entity);
}

TEST(MultiComponentView, ReverseIterator) {
    entt::registry registry;
    const auto entity = registry.create();
    registry.emplace<int>(entity);
    registry.emplace<char>(entity);

    const auto view = registry.view<int, char>();
    using iterator = typename decltype(view)::reverse_iterator;

    iterator end{view.rbegin()};
    iterator begin{};
    begin = view.rend();
    std::swap(begin, end);

    ASSERT_EQ(begin, view.rbegin());
    ASSERT_EQ(end, view.rend());
    ASSERT_NE(begin, end);

    ASSERT_EQ(begin++, view.rbegin());
    ASSERT_EQ(begin--, view.rend());

    ASSERT_EQ(++begin, view.rend());
    ASSERT_EQ(--begin, view.rbegin());

    ASSERT_EQ(*begin, entity);
    ASSERT_EQ(*begin.operator->(), entity);
}

TEST(MultiComponentView, Contains) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<int>(e0);
    registry.emplace<char>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    registry.destroy(e0);

    auto view = registry.view<int, char>();

    ASSERT_FALSE(view.contains(e0));
    ASSERT_TRUE(view.contains(e1));
}

TEST(MultiComponentView, Empty) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<double>(e0);
    registry.emplace<int>(e0);
    registry.emplace<float>(e0);

    const auto e1 = registry.create();
    registry.emplace<char>(e1);
    registry.emplace<float>(e1);

    auto view = registry.view<char, int, float>();

    ASSERT_EQ(view.size_hint(), 1u);
    ASSERT_EQ(view.begin(), view.end());
    ASSERT_EQ(view.rbegin(), view.rend());
}

TEST(MultiComponentView, Each) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<int>(e0);
    registry.emplace<char>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    auto view = registry.view<int, char>();
    auto cview = std::as_const(registry).view<const int, const char>();
    std::size_t cnt = 0;

    view.each([&cnt](auto, int &, char &) { ++cnt; });
    view.each([&cnt](int &, char &) { ++cnt; });

    for(auto &&[entt, iv, cv]: view.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), char &>);
        ++cnt;
    }

    ASSERT_EQ(cnt, std::size_t{6});

    cview.each([&cnt](auto, const int &, const char &) { --cnt; });
    cview.each([&cnt](const int &, const char &) { --cnt; });

    for(auto &&[entt, iv, cv]: cview.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), const int &>);
        static_assert(std::is_same_v<decltype(cv), const char &>);
        --cnt;
    }

    ASSERT_EQ(cnt, std::size_t{0});
}

TEST(MultiComponentView, EachWithSuggestedType) {
    entt::registry registry;

    for(auto i = 0; i < 3; ++i) {
        const auto entity = registry.create();
        registry.emplace<int>(entity, i);
        if(i) { registry.emplace<char>(entity); }
    }

    // makes char a better candidate during iterations
    const auto entity = registry.create();
    registry.emplace<int>(entity, 99);

    auto view = registry.view<int, char>();

    view.each<int>([value = view.size_hint()](const auto curr, const auto) mutable {
        ASSERT_EQ(curr, value--);
    });

    registry.sort<int>([](const auto lhs, const auto rhs) {
        return lhs < rhs;
    });

    view.each<int>([value = 0](const auto curr, const auto) mutable {
        ASSERT_EQ(curr, ++value);
    });

    registry.sort<int>([](const auto lhs, const auto rhs) {
        return lhs > rhs;
    });

    auto value = view.size_hint();

    for(auto &&curr: view.all()) {
        ASSERT_EQ(std::get<1>(curr), static_cast<int>(value--));
    }

    registry.sort<int>([](const auto lhs, const auto rhs) {
        return lhs < rhs;
    });

    for(auto &&curr: view.all()) {
        ASSERT_EQ(std::get<1>(curr), static_cast<int>(++value));
    }
}

TEST(MultiComponentView, EachWithHoles) {
    entt::registry registry;

    const auto e0 = registry.create();
    const auto e1 = registry.create();
    const auto e2 = registry.create();

    registry.emplace<char>(e0, '0');
    registry.emplace<char>(e1, '1');

    registry.emplace<int>(e0, 0);
    registry.emplace<int>(e2, 2);

    auto view = registry.view<char, int>();

    view.each([e0](auto entity, const char &c, const int &i) {
        ASSERT_EQ(entity, e0);
        ASSERT_EQ(c, '0');
        ASSERT_EQ(i, 0);
    });

    for(auto &&curr: view.all()) {
        ASSERT_EQ(std::get<0>(curr), e0);
        ASSERT_EQ(std::get<1>(curr), '0');
        ASSERT_EQ(std::get<2>(curr), 0);
    }
}

TEST(MultiComponentView, ConstNonConstAndAllInBetween) {
    entt::registry registry;
    auto view = registry.view<int, const char>();

    ASSERT_EQ(view.size_hint(), decltype(view.size_hint()){0});

    const auto entity = registry.create();
    registry.emplace<int>(entity, 0);
    registry.emplace<char>(entity, 'c');

    ASSERT_EQ(view.size_hint(), decltype(view.size_hint()){1});

    static_assert(std::is_same_v<decltype(view.get<int>({})), int &>);
    static_assert(std::is_same_v<decltype(view.get<const char>({})), const char &>);
    static_assert(std::is_same_v<decltype(view.get<int, const char>({})), std::tuple<int &, const char &>>);

    view.each([](auto &&i, auto &&c) {
        static_assert(std::is_same_v<decltype(i), int &>);
        static_assert(std::is_same_v<decltype(c), const char &>);
    });

    for(auto &&[entt, iv, cv]: view.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), const char &>);
    }
}

TEST(MultiComponentView, Find) {
    entt::registry registry;
    auto view = registry.view<int, const char>();

    const auto e0 = registry.create();
    registry.emplace<int>(e0);
    registry.emplace<char>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    const auto e2 = registry.create();
    registry.emplace<int>(e2);
    registry.emplace<char>(e2);

    const auto e3 = registry.create();
    registry.emplace<int>(e3);
    registry.emplace<char>(e3);

    registry.remove<int>(e1);

    ASSERT_NE(view.find(e0), view.end());
    ASSERT_EQ(view.find(e1), view.end());
    ASSERT_NE(view.find(e2), view.end());
    ASSERT_NE(view.find(e3), view.end());

    auto it = view.find(e2);

    ASSERT_EQ(*it, e2);
    ASSERT_EQ(*(++it), e3);
    ASSERT_EQ(*(++it), e0);
    ASSERT_EQ(++it, view.end());
    ASSERT_EQ(++view.find(e0), view.end());

    const auto e4 = registry.create();
    registry.destroy(e4);
    const auto e5 = registry.create();
    registry.emplace<int>(e5);
    registry.emplace<char>(e5);

    ASSERT_NE(view.find(e5), view.end());
    ASSERT_EQ(view.find(e4), view.end());
}

TEST(MultiComponentView, ExcludedComponents) {
    entt::registry registry;

    const auto e0 = registry.create();
    registry.emplace<int>(e0, 0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1, 1);
    registry.emplace<char>(e1);

    const auto e2 = registry.create();
    registry.emplace<int>(e2, 2);

    const auto e3 = registry.create();
    registry.emplace<int>(e3, 3);
    registry.emplace<char>(e3);

    const auto view = std::as_const(registry).view<const int>(entt::exclude<char>);

    for(const auto entity: view) {
        ASSERT_TRUE(entity == e0 || entity == e2);

        if(entity == e0) {
            ASSERT_EQ(view.get<const int>(e0), 0);
        } else if(entity == e2) {
            ASSERT_EQ(view.get<const int>(e2), 2);
        }
    }

    registry.emplace<char>(e0);
    registry.emplace<char>(e2);
    registry.remove<char>(e1);
    registry.remove<char>(e3);

    for(const auto entity: view) {
        ASSERT_TRUE(entity == e1 || entity == e3);

        if(entity == e1) {
            ASSERT_EQ(view.get<const int>(e1), 1);
        } else if(entity == e3) {
            ASSERT_EQ(view.get<const int>(e3), 3);
        }
    }
}

TEST(MultiComponentView, EmptyTypes) {
    entt::registry registry;

    const auto entity = registry.create();
    registry.emplace<int>(entity);
    registry.emplace<char>(entity);
    registry.emplace<double>(entity);
    registry.emplace<empty_type>(entity);

    const auto other = registry.create();
    registry.emplace<int>(other);
    registry.emplace<char>(other);

    auto view1 = registry.view<int, char, empty_type>();
    auto view2 = registry.view<int, empty_type, char>();
    auto view3 = registry.view<empty_type, int, char>();
    auto view4 = registry.view<int, char, double>();

    view1.each([entity](const auto entt, int, char) {
        ASSERT_EQ(entity, entt);
    });

    for(auto &&[entt, iv, cv]: view1.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), char &>);
        ASSERT_EQ(entity, entt);
    }

    view2.each([check = true](int, char) mutable {
        ASSERT_TRUE(check);
        check = false;
    });

    for(auto &&[entt, iv, cv]: view2.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), char &>);
        ASSERT_EQ(entity, entt);
    }

    view3.each([entity](const auto entt, int, char) {
        ASSERT_EQ(entity, entt);
    });

    for(auto &&[entt, iv, cv]: view3.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), char &>);
        ASSERT_EQ(entity, entt);
    }

    view3.each<empty_type>([entity](const auto entt, int, char) {
        ASSERT_EQ(entity, entt);
    });

    view2.each<empty_type>([check = true](int, char) mutable {
        ASSERT_TRUE(check);
        check = false;
    });

    view4.each([entity](const auto entt, int, char, double) {
        ASSERT_EQ(entity, entt);
    });

    for(auto &&[entt, iv, cv, dv]: view4.all()) {
        static_assert(std::is_same_v<decltype(entt), entt::entity>);
        static_assert(std::is_same_v<decltype(iv), int &>);
        static_assert(std::is_same_v<decltype(cv), char &>);
        static_assert(std::is_same_v<decltype(dv), double &>);
        ASSERT_EQ(entity, entt);
    }
}

TEST(MultiComponentView, FrontBack) {
    entt::registry registry;
    auto view = registry.view<const int, const char>();

    ASSERT_EQ(view.front(), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(view.back(), static_cast<entt::entity>(entt::null));

    const auto e0 = registry.create();
    registry.emplace<int>(e0);
    registry.emplace<char>(e0);

    const auto e1 = registry.create();
    registry.emplace<int>(e1);
    registry.emplace<char>(e1);

    const auto entity = registry.create();
    registry.emplace<char>(entity);

    ASSERT_EQ(view.front(), e1);
    ASSERT_EQ(view.back(), e0);
}
