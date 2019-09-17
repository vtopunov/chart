#pragma once

#include <core/intrusive_list.h>
#include <core/assert.h>

class object
{
public:
    constexpr object( object* parent = nullptr ) noexcept
        : parent_{ parent }
        , siblings_{ this, this }
        , childrens_{ nullptr }
    {
        if ( parent_ )
        {
            if ( parent_->childrens_ )
            {
                siblings_ = siblings_impl_.push( parent_->childrens_, this );
            }
            parent_->childrens_ = this;
        }
    }

    constexpr void replace_owwer( object* new_owner ) noexcept
    {
        siblings_impl_.pop( this );
        new_owner->ownership_assignment( this );
    }

    constexpr virtual void close() noexcept
    {
        destroy_childrens();
        destroy_from_siblings();
        destroy_from_parent();
    }

private:
    constexpr void ownership_assignment( const object* old_owner ) noexcept
    {
        childrens_ = old_owner->childrens_;
        parent_ = old_owner->parent_;
        siblings_ = old_owner->siblings_;

        if ( siblings_.prev == old_owner )
        {
            siblings_.prev = this;
        }

        if ( siblings_.next == old_owner )
        {
            siblings_.next = this;
        }

        if ( parent_ )
        {
            assert( parent_->childrens_ );

            if ( parent_->childrens_ == old_owner )
            {
                parent_->childrens_ = this;
            }

            siblings_ = siblings_impl_.push( parent_->childrens_, this );
        }

        if ( childrens_ )
        {
            auto item = childrens_;
            do
            {
                if ( item->parent_ == old_owner )
                {
                    item->parent_ = this;
                }

                item = siblings_impl_.next( item );
            }
            while ( item != childrens_ );
        }
    }

    constexpr void destroy_childrens() noexcept
    {
        if ( const auto childrens = release_childrens() )
        {
            auto item = childrens;
            do
            {
                item->close();
                item = siblings_impl_.next( item );
            }
            while ( item != childrens );
        }
    }

    constexpr void destroy_from_parent() noexcept
    {
        if ( const auto parent = release_parent() )
        {
            assert( parent->childrens_ );
            if ( parent->childrens_ == this )
            {
                if ( siblings_.next == this )
                {
                    parent->childrens_ = nullptr
                }
                else
                {
                    parent->childrens_ = siblings_.next;
                }
            }
        }
    }

    constexpr void destroy_from_siblings() noexcept
    {
        siblings_impl_.pop( this );
    }

    constexpr object* release_childrens() noexcept
    {
        const auto temp = childrens_;
        childrens_ = nullptr;
        return temp;
    }

    constexpr object* release_parent() noexcept
    {
        const auto temp = parent_;
        parent_ = nullptr;
        return temp;
    }

private:
    object* parent_;
    object* childrens_;
    intrusive_node<object> siblings_;

    static constexpr intrusive_list_impl<object> siblings_impl_{ &object::siblings_ };
};

