from collections.abc import Iterable, Generator


def flatten(iterable: Iterable) -> Generator:
    for item in iterable:
        if isinstance(item, str):
            yield item
        elif isinstance(item, Iterable):
            yield from flatten(item)
        else:
            yield item
