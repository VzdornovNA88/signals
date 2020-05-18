/**
  ******************************************************************************
  * @file             storage.hpp
  * @brief
  * @author           Nik A. Vzdornov (VzdornovNA88@yandex.ru)
  * @date             10.09.19
  * @copyright
  *
  * Copyright (c) 2019 VzdornovNA88
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  *
  ******************************************************************************
  */

#ifndef _STORAGE_
#define _STORAGE_

#include <variant>
#include <type_traits>

namespace signaler::detail {

	class storage_t final {

		struct control_t { size_t cnt = 0; };
		template< typename T >
		struct control_block_t : control_t {

			T payload;
			control_block_t(T&& _payload) : payload(_payload) {}
		};

		enum storage_state_t : size_t {
			INVALID = 0,
			LOCAL = 1,
			DYNAMIC = 2,
		};

		struct small_object_t {
			std::byte _array[16];
		};

		using storage_small_object_t = std::aligned_storage<sizeof(small_object_t), alignof(small_object_t)>::type;
		using storage_big_object_t = void*;

		std::variant<std::monostate, storage_small_object_t, storage_big_object_t> store;


		void destructor(void* p_store) {

			if (p_store == nullptr) return;

			auto& cnt_ref = static_cast<control_t*>(p_store)->cnt;

			if (cnt_ref == 0) {

				delete p_store;
				p_store = nullptr;
			}
			else {
				--cnt_ref;
				p_store = nullptr;
			}
		}

	public:

		storage_t() = default;
		storage_t(std::nullptr_t const) : storage_t() {}
		~storage_t() {
			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);
		}

		storage_t(storage_t const& s) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);

			store = s.store;

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				if (*p_store != nullptr)
					++static_cast<control_t*>(*p_store)->cnt;
		}

		storage_t(storage_t&& s) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);

			store = std::move(s.store);

			if (auto p_store = std::get_if<DYNAMIC>(&s.store)) {
				*p_store = nullptr;
				s.store = std::monostate{};
			}
		}

		storage_t& operator = (storage_t const& s) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);

			store = s.store;

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				if (*p_store != nullptr)
					++static_cast<control_t*>(*p_store)->cnt;

			return *this;
		}

		storage_t& operator = (storage_t&& s) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);

			store = std::move(s.store);

			if (auto p_store = std::get_if<DYNAMIC>(&s.store)) {
				*p_store = nullptr;
				s.store = std::monostate{};
			}

			return *this;
		}

		storage_t& operator = (std::nullptr_t const null_object) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);
			return *this;
		}

		storage_t& operator = (int const null_object) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);
			return *this;
		}


		template< typename T >
		constexpr auto init(T&& f) {

			if (auto p_store = std::get_if<DYNAMIC>(&store))
				destructor(*p_store);

			using functor_t = typename std::decay<T>::type;

			if constexpr (sizeof(functor_t) > sizeof(small_object_t)) {
				store = new control_block_t<functor_t>(std::forward<T>(f));
				return &static_cast<control_block_t<functor_t>*>(*std::get_if<DYNAMIC>(&store))->payload;
			}
			else {
				store = storage_small_object_t();
				auto p_store = std::get_if<LOCAL>(&store);
				new (p_store) functor_t(std::forward<T>(f));
				return p_store;
			}
		}
	};

}

#endif  //_STORAGE_