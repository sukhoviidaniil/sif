/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-20
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include "sif/event/Event_Bus.h"

namespace sif::event {
    Event_Bus::Subscription::~Subscription()  {
        unsubscribe();
    }

    Event_Bus::Subscription::Subscription(
        std::shared_ptr<Event_Bus> bus,
        const std::type_index type,
        const HandlerId id
        ) : bus_(std::move(bus)), type_(type), id_(id){
    }

    void Event_Bus::Subscription::unsubscribe()  {
        if (bus_) {
            bus_->unsubscribe(type_, id_);
            bus_ = nullptr;
        }
    }

    Event_Bus::Subscription::Subscription(Subscription&& other) noexcept
    : bus_(std::move(other.bus_)), type_(other.type_), id_(other.id_) {
        other.bus_ = nullptr;
    }

    Event_Bus::Subscription & Event_Bus::Subscription::operator=(Subscription &&other) noexcept {
        bus_  = std::move(other.bus_);
        type_ = other.type_;
        id_   = other.id_;
        other.bus_ = nullptr;
        return *this;
    }

    void Event_Bus::unsubscribe(std::type_index type, HandlerId id) {
        auto it = handlers_.find(type);
        if (it == handlers_.end()) return;

        auto& list = it->second;
        list.erase(
            std::remove_if(list.begin(), list.end(),
                [id](const Handler& h) { return h.id == id; }),
            list.end()
        );
    }

    void Event_Bus::sort(std::vector<Handler> &list) {
        std::sort(list.begin(), list.end(),
            [](const Handler& a, const Handler& b) {
                return a.priority > b.priority;
            });
    }

    void Event_Bus::emit(const EventConcept& event) {
        auto it = handlers_.find(event.type());
        if (it == handlers_.end())
            return;

        for (const Handler& h : it->second) {
            if ((h.mask & event.mask()) != EventMask::None) {
                h.fn(event);
            }
        }
    }
}