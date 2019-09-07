#pragma once



class object
{
public:
    constexpr object( object* parent ) noexcept
    {}


    virtual void close() noexcept
    {
        const auto prev = prev_sibling;
        const auto next = next_sibling;
        if ( next == this )
        {
            if ( parent )
            {
                parent->children = nullptr;
            }
        }
        else
        {
            prev->next_sibling = next;
            next->prev_sibling = prev;
        }
    }

private:
    object* parent;
    object* children;
    object* prev_sibling;
    object* next_sibling;
};
