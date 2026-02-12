class EnhancedList(list):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    @property
    def first(self):
        return self[0]
    @property
    def last(self):
        return self[-1]
    @property
    def size(self):
        return len(self)
    @first.setter
    def first(self, value):
        self[0] = value
    @last.setter
    def last(self, value):
        self[-1] = value
    @size.setter
    def size(self, sz):
        cursz = len(self)
        if sz > cursz:
            self.extend([None] * (sz - cursz))
        elif sz < cursz:
            del self[sz:]
