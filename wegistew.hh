/* SPDX-License-Identifier: BSD-3-Clause */
/* wegistew v0.0.4 - https://github.com/lethalbit/wegistew-cc */
/* wegistew.hh - Compile-time register grokking */
#pragma once
#if !defined(WEGISTEW_HH)
#define WEGISTEW_HH

#if __cplusplus < 201402L
	#error "wegistew requires C++14 or newer"
#else

#include <type_traits>
#include <limits>
#include <cstdint>

namespace wegistew {
	/* A compile_time generated bitmask for register fields */
	template<std::size_t _lsb, std::size_t _msb>
	struct bitspan_t final {
		static constexpr std::size_t size = (_msb - _lsb) + 1;
		static_assert(_lsb <= _msb, "bitspan LSB must be smaller than or equal to the MSB");

		/* Internal register field machinery */
		template<typename T, std::uintptr_t address, std::size_t _msb_ = _msb, std::size_t _lsb_ = _lsb>
		struct memaddr_field final {
			using value_type = T;
			using vu_type = typename std::make_unsigned_t<value_type>;

			static constexpr auto msb = _msb_;
			static constexpr auto lsb = _lsb_;
			static constexpr std::size_t width = std::numeric_limits<vu_type>::digits;
			static_assert(msb <= width, "MSB must be less than or equal to the width of the bitspan type");
			static constexpr vu_type computed_mask = (((vu_type(1) << (vu_type(msb) + vu_type(1)) - vu_type(lsb)) - vu_type(1)) << vu_type(lsb));

			static constexpr std::uintptr_t addr = address;

			/* Get the value of this field in the given register */
			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr
			std::enable_if_t<!std::is_enum<V>::value, V> get() noexcept {
				const auto reg =  reinterpret_cast<volatile T*>(addr);

				return (vu_type((*reg) & computed_mask) >> lsb);
			}

			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr
			std::enable_if_t<std::is_enum<V>::value, V> get() noexcept {
				const auto reg =  reinterpret_cast<volatile T*>(addr);

				return static_cast<V>(
					vu_type((*reg) & computed_mask) >> lsb
				);
			}

			/* Set the value of this field in the given register */
            template<typename V>
			static inline constexpr
            std::enable_if_t<!std::is_enum<V>::value> set(const V v) noexcept {
				const auto reg =  reinterpret_cast<volatile V*>(addr);
				
				// If masking out something in the size of the type being written results in a complete overwrite, skip the unnecessary read
				if constexpr (V(~computed_mask) == 0) {
					(*reg) = (vu_type(v) << lsb) & computed_mask;
				} else {
					(*reg) = (((*reg) & ~computed_mask) | ((vu_type(v) << lsb) & computed_mask));
				}
			}

            template<typename V>
			static inline constexpr
            std::enable_if_t<std::is_enum<V>::value> set(const V v) noexcept {
                using Vt = typename std::underlying_type_t<V>;
				const auto reg = reinterpret_cast<volatile Vt*>(addr);
				
				// If masking out something in the size of the type being written results in a complete overwrite, skip the unnecessary read
				if constexpr (Vt(~computed_mask) == 0) {
					(*reg) = (vu_type(v) << lsb) & computed_mask;
				} else {
					(*reg) = (((*reg) & ~computed_mask) | ((vu_type(v) << lsb) & computed_mask));
				}
			}
		};

		/* Value field machinery */
		template<typename T, std::size_t _msb_ = _msb, std::size_t _lsb_ = _lsb>
		struct field final {
			using value_type = T;
			using vu_type = typename std::make_unsigned_t<value_type>;

			static constexpr auto msb = _msb_;
			static constexpr auto lsb = _lsb_;
			static constexpr std::size_t width = std::numeric_limits<vu_type>::digits;
			static_assert(msb <= width, "MSB must be less than or equal to the width of the bitspan type");
			static constexpr vu_type computed_mask = (((vu_type(1) << (vu_type(msb) + vu_type(1)) - vu_type(lsb)) - vu_type(1)) << vu_type(lsb));


			/* Get the value of this field in the given register */
			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr
			std::enable_if_t<!std::is_enum<V>::value, V> get(const V v) noexcept {
				return (vu_type((v) & computed_mask) >> lsb);
			}

			template<typename V = vu_type>
			[[nodiscard]]
			static inline constexpr
			std::enable_if_t<std::is_enum<V>::value, V> get(const V v) noexcept {
				return static_cast<V>(
					vu_type((v) & computed_mask) >> lsb
				);
			}

			/* Set the value of this field in the given register */
            template<typename V>
			static inline constexpr
            void set(V& f, const V v) noexcept {
				f = (((f) & ~computed_mask) | ((vu_type(v) << lsb) & computed_mask));
			}
		};
	};

	/* A "specialization" of genmask_t for a single bit field */
	template<std::size_t idx>
	using bit_t = bitspan_t<idx, idx>;

	namespace {
		template<std::size_t, typename...>
		struct type_at_index_t;

		template<std::size_t N, typename T, typename... U>
		struct type_at_index_t<N, T, U...> {
			using type = typename type_at_index_t<N - 1, U...>::type;
		};

		template<typename T, typename... U>
		struct type_at_index_t<0, T, U...> {
			using type = T;
		};
	}

	/* Compiletime / runtime register access */
	template<typename T, uintptr_t address, typename... U>
	struct wegistew_t final {
		using value_type = T;
		using vu_type = typename std::make_unsigned_t<value_type>;

		static constexpr auto width = std::numeric_limits<T>::digits;
		static constexpr auto size = width;
		static constexpr std::uintptr_t addr = address;
		static constexpr std::size_t field_count = sizeof...(U);

		/* Returns the field requested by index */
		template<std::size_t idx>
		using field = typename type_at_index_t<idx, U...>::type::template memaddr_field<T, address>;

		/* This is functionally equivalent to ::fields<idx>::get() */
		template<std::size_t idx, typename V = vu_type>
		[[nodiscard]]
		static inline constexpr auto get() noexcept {
			static_assert(idx < field_count, "field index out of range");
			return field<idx>::template get<V>();
		}

		/* This is functionally equivalent to ::fields<idx>::set(v) */
		template<std::size_t idx, typename V>
		static inline constexpr void set(const V v) noexcept {
			static_assert(idx < field_count, "field index out of range");
			field<idx>::set(v);
		}

		/* register-wide getter / setter */
		[[nodiscard]]
		inline volatile T& operator*() const noexcept {
			const auto reg =  reinterpret_cast<volatile T*>(addr);
			return *reg;
		}

		[[nodiscard]]
		const volatile inline T& operator*() noexcept {
			const auto reg =  reinterpret_cast<volatile T*>(addr);
			return *reg;
		}

	};

	/* Like wegistew_t but for dealing with values passed in rather than memory mapped registers */
	template<typename T, typename... U>
	struct bitfield_t final {
		using value_type = T;
		using vu_type = typename std::make_unsigned_t<value_type>;

		static constexpr auto width = std::numeric_limits<T>::digits;
		static constexpr auto size = width;
		static constexpr std::size_t field_count = sizeof... (U);

		/* Returns the field requested by index */
		template<std::size_t idx>
		using field = typename type_at_index_t<idx, U...>::type::template field<T>;

		/* This is functionally equivalent to ::fields<idx>::get() */
		template<std::size_t idx, typename V = vu_type>
		[[nodiscard]]
		static inline constexpr auto get(const V v) noexcept {
			static_assert(idx < field_count, "field index out of range");
			return field<idx>::template get<V>(v);
		}

		/* This is functionally equivalent to ::fields<idx>::set(v) */
		template<std::size_t idx, typename V>
		static inline constexpr void set(V& f, const V v) noexcept {
			static_assert(idx < field_count, "field index out of range");
			field<idx>::set(f, v);
		}

	};
}
#endif

#endif /* WEGISTEW_HH */
