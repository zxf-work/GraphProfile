import igraph as ig, random, sys, time
random.seed(7)
def write(g, path, directed):
    with open(path, "w") as f:
        for e in g.es:
            f.write(f"{e.source}\t{e.target}\n")
t=time.time()
g = ig.Graph.Barabasi(n=2_000_000, m=5, directed=False); write(g, "ba_10m.txt", False); print("ba_10m", g.vcount(), g.ecount(), round(time.time()-t)); del g
t=time.time()
g = ig.Graph.Barabasi(n=10_000_000, m=5, directed=False); write(g, "ba_50m.txt", False); print("ba_50m", g.vcount(), g.ecount(), round(time.time()-t)); del g
t=time.time()
g = ig.Graph.Barabasi(n=3_000_000, m=4, directed=True); write(g, "ba_12m_directed.txt", True); print("ba_12m_dir", g.vcount(), g.ecount(), round(time.time()-t))
