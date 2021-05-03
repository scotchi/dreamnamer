#pragma once

struct Episode
{
    Episode(int s, int e) : season(s), episode(e) {}
    int season = 0;
    int episode = 0;
};

