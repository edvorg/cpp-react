/*

  react LGPL Source Code
  Copyright (C) 2014 Edward Knyshov

  This file is part of the react LGPL Source Code (react Source Code).

  react Source Code is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  react Source Code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with react Source Code. If not, see <http://www.gnu.org/licenses/>

*/

#pragma once

#ifndef LINK_HPP
#define LINK_HPP

#include <tuple/tuple.hpp>
#include "rx.hpp"
#include "rxrelaxed.hpp"

namespace react {

    template <class T, class ... TS>
    class Rx;

    template <class ... TS>
    class Link {
    public:
        using Tuple = tuple::Tuple<const Var<TS> * ...>;

        Link() = default;

        Link(const Link & l):
            vars(l.vars) {
        }

        Link(Link && l):
            vars(std::move(l.vars)) {
        }

        Link(const Tuple & newVars):
            vars(newVars) {
        }

        Link(Tuple && newVars):
            vars(std::move(newVars)) {
        }

        Link(const Var<TS> & ... newVars):
            vars(&newVars ...){
        }

        Link & operator=(const Link & l) {
            vars = l.vars;
            return *this;
        }

        Link & operator=(Link && l) {
            vars = std::move(l.vars);
            return *this;
        }

        const auto & getVars() const {
            return vars;
        }

        template <class FN>
        auto rx(FN && f) {
            using RxType = Rx<decltype(f(ref<TS>() ...)), TS ...>;
            return RxType(std::forward<FN>(f), *this);
        }

        template <class FN>
        auto rxRelaxed(FN && f) {
            using RxType = RxRelaxed<decltype(f(ref<TS>() ...))>;
            return RxType(std::forward<FN>(f), *this);
        }

        template <class RX, class FN>
        void reconnect(RX & rx, FN && f) {
            rx.reconnect(std::forward<FN>(f), *this);
        }

    private:
        template <class T>
        static auto & ref() {
            return *static_cast<T *>(nullptr);
        }

        Tuple vars;
    };

    template <class ... TS>
    auto link(const Var<TS> & ... newVars) {
        return Link<TS ...>(newVars ...);
    }

}

#endif // LINK_HPP
