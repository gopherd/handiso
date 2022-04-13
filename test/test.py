#!/usr/bin/env python
# -*- coding: utf-8 -*-

import random

import handiso

SUITS = 4
RANK = 13
CARDS = 13*4

"""
def get_suit(card):
    return card&3

def get_rank(card):
    return card>>2

def make_card(suit, rank):
    return rank<<2 | suit

def nth_bit(used, bit):
    # TODO
    return 0

def test_full(indexer):
    rounds = indexer.rounds()

    total_cards = 0
    for i in range(rounds):
        total_cards += indexer.cards(i)
    assert total_cards <= 7

    permutations = 1
    for i in range(total_cards):
        permutations *= CARDS-i
    
    round_size = indexer.round_size(rounds - 1)
    seen = {}
    cards = [0 for _ in range(total_cards)]

    for i in range(permutations):
        p = i
        used = 0
        for j in range(total_cards):
            cards[j] = nth_bit(~used, p%(CARD-j))
            p /= CARDS-j
            used |= 1<<cards[j]

        index = indexer.index_last(cards)
        assert index < round_size
        if index in seen:
            seen[index] = seen[index] + 1
        else:
            seen[index] = 1

    for i in range(round_size):
        got_cards = indexer.unindex(rounds - 1, i)
        assert indexer.index_last(got_cards) == i

def test_random(indexer):
    rounds = indexer.rounds()

    total_cards = 0
    for i in range(rounds):
        total_cards += indexer.cards(i)
    assert total_cards <= 7

    round_size = indexer.round_size(rounds - 1)

    deck = [i for i in range(CARDS)]
    pi = [i for i in range(SUITS)]
    cards = [0 for _ in range(total_cards)]

    for _ in range(10000000):
        random.shuffle(deck)
        random.shuffle(pi)
        for i in range(total_cards):
            cards[i] = make_card(pi[get_suit(deck[i])], get_rank(deck[i]))
        
        i = 0
        j = 0
        while i < total_cards:
            num_cards = indexer.cards(j)
            for k in range(num_cards):
                i++
                ii = random.randrange(num_cards - k)
                tmp = cards[i]
                cards[i] = cards[i+ii]
                cards[i+ii] = tmp
            j++

        index = indexer.index_last(deck)
        idnex2 = indexer.index_last(cards)
        assert index < size
        assert index < index2

        got_cards = indexer.unindex(rounds - 1, index)
        assert indexer.index_last(got_cards) == index
"""

def main():
    public_indexer = handiso.Indexer([3])
    preflop_indexer = handiso.Indexer([2])
    flop_indexer = handiso.Indexer([2, 3])
    turn_indexer = handiso.Indexer([2, 3, 1])
    river_indexer = handiso.Indexer([2, 3, 1, 1])

    print("sizes: public=%d, preflop=%d, flop=%d, turn=%d, river=%d" % \
          (public_indexer.size(0),
           river_indexer.size(0),
           river_indexer.size(1),
           river_indexer.size(2),
           river_indexer.size(3)))

    print("configurations: %d %d %d %d" % \
          (river_indexer.configurations(0),
           river_indexer.configurations(1),
           river_indexer.configurations(2),
           river_indexer.configurations(3)))

    print("permutations: %d %d %d %d" % \
          (river_indexer.permutations(0),
           river_indexer.permutations(1),
           river_indexer.permutations(2),
           river_indexer.permutations(3)))

    assert public_indexer.size(0) == 1755
    assert preflop_indexer.size(0) == 169
    assert flop_indexer.size(0) == 169
    assert turn_indexer.size(0) == 169
    assert river_indexer.size(0) == 169
    assert flop_indexer.size(1) == 1286792
    assert turn_indexer.size(1) == 1286792
    assert river_indexer.size(1) == 1286792
    assert turn_indexer.size(2) == 55190538
    assert river_indexer.size(2) == 55190538
    assert river_indexer.size(3) == 2428287420

if __name__ == "__main__":
    main()
