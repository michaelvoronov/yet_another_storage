#pragma once
#include <unordered_map>
#include <memory>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class AhoCorasickEngine {
  using StringViewType = std::basic_string_view<CharType>;
  template<typename CharType>
  struct Node;
  using CharNode = Node<CharType>;

 public:
  static_assert(std::is_trivially_copyable_v<LeafType>, "LeafType should be POD");

  AhoCorasickEngine()
      : trie_(new CharNode()),
        non_exist_leaf_type_(LeafType::MakeNonExistType())
  {}
  ~AhoCorasickEngine() = default;
  AhoCorasickEngine(AhoCorasickEngine &&other) noexcept 
      : root_(std::move(other.root_)),
        non_exist_leaf_type_(LeafType::MakeNonExistType())
  {}

  bool Insert(StringViewType key, LeafType &leaf) {
    auto current = trie_.get();

    for (const auto &ch : key) {
      auto next = getNextNode(ch, current);
      if (nullptr == next) {
        current->routes_[ch].reset(new CharNode());
        current = current->routes_[ch].get();
        continue;
      }
      current = next;
    }
    current->leaf_= std::move(leaf);
    return true;
  }

  LeafType& Get(StringViewType key) noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? non_exist_leaf_type_ : node->leaf_;
  }

  const LeafType& Get(StringViewType key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? non_exist_leaf_type_ : node->leaf_;
  }

  bool Delete(StringViewType key) {
    const auto node = getPathNode(key);
    if (nullptr == node) {
      return false;
    }

    node->leaf_ = non_exist_leaf_type_;
    return true;
  }

  bool HasKey(StringViewType key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? false : non_exist_leaf_type_ != node->leaf_;
  }

  AhoCorasickEngine(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(AhoCorasickEngine&&) = delete;

private:
  template<typename CharType>
  struct Node {
    Node() : leaf_(LeafType::MakeNonExistType()) {}
    std::unordered_map<CharType, std::unique_ptr<Node>> routes_;
    LeafType leaf_;
  };

  LeafType non_exist_leaf_type_;
  std::unique_ptr<CharNode> trie_;

  CharNode *getNextNode(CharType ch, CharNode *current) const noexcept {
    if (nullptr == current || !current->routes_.count(ch)) {
      // the next node hasn't been created yet
      return nullptr;
    }
    return current->routes_[ch].get();
  } 

  CharNode *getPathNode(StringViewType key) const noexcept {
    auto current = trie_.get();

    for (const auto &ch : key) {
      current = getNextNode(ch, current);
      if (nullptr == current) {
        return nullptr;
      }
    }
    return current;
  }
};

} // namespace index_helper
} // namespace yas