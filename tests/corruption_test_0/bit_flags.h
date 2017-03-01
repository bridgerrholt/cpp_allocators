#ifndef BRIDGERRHOLT_BIT_FLAGS_H
#define BRIDGERRHOLT_BIT_FLAGS_H

namespace bridgerrholt {

template <class t_EnumType, class NumberType = int>
class EnumWrapper
{
	public:
		using EnumType = t_EnumType;

		constexpr EnumWrapper(EnumType valueSet) :
			value {valueSet} {}

		constexpr EnumWrapper(NumberType valueSet) :
			value {static_cast<EnumType>(valueSet)} {}

		EnumType value;

		constexpr NumberType getNumeric() const {
			return castEnum(value);
		}

		constexpr NumberType & getNumeric() {
			return *reinterpret_cast<NumberType*>(&value);
		}


		static constexpr NumberType castEnum(EnumType value) {
			return static_cast<NumberType>(value);
		}

		friend constexpr EnumWrapper operator~(EnumWrapper object) {
			return {~object.getNumeric()};
		}

		friend constexpr EnumWrapper operator!(EnumWrapper object) {
			return {!object.getNumeric()};
		}


		friend constexpr EnumWrapper operator|(EnumWrapper first,
		                                       EnumWrapper second) {
			return {first.getNumeric() | second.getNumeric()};
		}

		friend constexpr EnumWrapper operator&(EnumWrapper first,
		                                       EnumWrapper second) {
			return {first.getNumeric() & second.getNumeric()};
		}

		friend constexpr EnumWrapper operator^(EnumWrapper first,
		                                       EnumWrapper second) {
			return {first.getNumeric() ^ second.getNumeric()};
		}


		friend constexpr EnumWrapper operator|=(EnumWrapper & first,
		                                        EnumWrapper   second) {
			first.getNumeric() |= second.getNumeric();
			return first;
		}

		friend constexpr EnumWrapper operator&=(EnumWrapper & first,
		                                        EnumWrapper second) {
			first.getNumeric() &= second.getNumeric();
			return first;
		}

		friend constexpr EnumWrapper operator^=(EnumWrapper & first,
		                                        EnumWrapper second) {
			first.getNumeric() ^= second.getNumeric();
			return first;
		}


		friend constexpr bool operator==(EnumWrapper first,
		                                 EnumWrapper second) {
			return (first.value == second.value);
		}

		friend constexpr bool operator!=(EnumWrapper first,
		                                 EnumWrapper second) {
			return !(first == second);
		}
};


template <class t_EnumType, class t_NumberType = int>
class BasicBitFlags
{
	public:
		using EnumType   = t_EnumType;
		using NumberType = t_NumberType;
		using EnumWrapperType = EnumWrapper<EnumType, NumberType>;

		BasicBitFlags(EnumWrapperType wrapper) : wrapper_ {wrapper} {}

		BasicBitFlags & set(EnumWrapperType value) {
			wrapper_ |= value;
			return *this;
		}

		BasicBitFlags & unset(EnumWrapperType value) {
			wrapper_ &= !value;
			return *this;
		}

		bool isSet(EnumWrapperType value) {
			return ((wrapper_ & value) == value);
		}


	private:
		EnumWrapperType wrapper_;
};

}

#endif
