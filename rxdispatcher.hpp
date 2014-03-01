#pragma once

#ifndef RXDISPATCHER_HPP
#define RXDISPATCHER_HPP

#include "vardispatcher.hpp"
#include "link.hpp"
#include "exceptions.hpp"
#include <unordered_map>

namespace react {

    template <class T>
    class Var;

    template <class ... TS>
    class Link;

    template <class T, class ... TS>
    class Rx;

    template <class T>
    class VarDispatcher;

    template <class T, class ... TS>
    class RxDispatcher {
    public:
        using RxT = Rx<T, TS ...>;
        using LinkT = Link<TS ...>;
        using RxesLinks = std::unordered_map<const RxT *, LinkT>;
        using Tuple = typename LinkT::Tuple;
        using AllIndices = typename Tuple::AllIndices;
        template <unsigned int ... INDICES>
        using Indices = tuple::Indices<INDICES ...>;
        template <unsigned int INDEX>
        using Accessor = tuple::Accessor<INDEX, const Var<TS> * ...>;
        using RxFunction = std::function<T(const TS & ...)>;
        using RxesFunctions = std::unordered_map<const RxT *, RxFunction>;

        static auto & instance() {
            static RxDispatcher dispatcher;
            return dispatcher;
        }

        template <class FN, class LINK>
        void connect(RxT & rx, FN && f, LINK && l) {
            set(rxesLinks, &rx, std::forward<LINK>(l));
            set(rxesFunctions, &rx, std::forward<FN>(f));
            connectListener(rx, l);
        }

        void disconnect(RxT & rx) {
            disconnectListener(rx, query(rxesLinks, &rx));
            erase(rxesFunctions, &rx);
            erase(rxesLinks, &rx);
        }

        T compute(const RxT & rx) const {
            return compute(rx, AllIndices{});
        }

        void updateLink(RxT & rx) {
            updateLink(rx, AllIndices{});
        }

    private:
        RxDispatcher() = default;

        template <unsigned int INDEX>
        const auto & var(const RxT & rx, const LinkT & l) const {
            return Accessor<INDEX>::Get(l.getVars());
        }

        // computing

        template <class U>
        const auto & value(const RxT & rx, const Var<U> * v) const {
            return VarDispatcher<U>::instance().value(v, rx);
        }

        template <unsigned int ... INDICES>
        T compute(const RxT & rx, const Indices<INDICES ...> &) const {
            auto & l = query(rxesLinks, &rx);
            auto & f = query(rxesFunctions, &rx);
            return f(value(rx, var<INDICES>(rx, l)) ...);
        }

        // link updating

        template <class U>
        const Var<U> & reincarnatedVar(const Var<U> * v, RxT & rx) {
            return VarDispatcher<U>::instance().reincarnatedVar(v, rx);
        }

        template <unsigned int ... INDICES>
        void updateLink(RxT & rx, const Indices<INDICES ...> &) {
            auto & l = query(rxesLinks, &rx);
            l = link(reincarnatedVar(var<INDICES>(rx, l), rx) ...);
        }

        // connecting

        template <class U>
        void connectListener(RxT & rx, const Link<U> & l) {
            const auto & var = l.getVars().GetFirst();
            VarDispatcher<U>::instance().connect(var, rx);
        }

        template <class U, class UU, class ... US>
        void connectListener(RxT & rx, const Link<U, UU, US ...> & l) {
            const auto & var = l.getVars().GetFirst();
            VarDispatcher<U>::instance().connect(var, rx);
            connectListener(rx, Link<UU, US ...>(*l.getVars().Next()));
        }

        // diconnecting

        template <class U>
        void disconnectListener(RxT & rx, const Link<U> & l) {
            const auto & var = l.getVars().GetFirst();
            VarDispatcher<U>::instance().disconnect(var, rx);
        }

        template <class U, class UU, class ... US>
        void disconnectListener(RxT & rx, const Link<U, UU, US ...> & l) {
            const auto & var = l.getVars().GetFirst();
            VarDispatcher<U>::instance().disconnect(var, rx);
            disconnectListener(rx, Link<UU, US ...>(*l.getVars().Next()));
        }

        RxesLinks rxesLinks;
        RxesFunctions rxesFunctions;
    };

}

#endif // RXDISPATCHER_HPP
