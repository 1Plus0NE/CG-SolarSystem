#!/usr/bin/env python3
"""
Conversor de OBJ para formato .3d

Formato .3d:
  - Linha 1: número total de vértices (3 por triângulo)
  - Linhas seguintes: x y z de cada vértice, sem índices,
    vértices expandidos (repetidos se partilhados entre faces)

Suporta:
  - Faces triangulares (f v1 v2 v3)
  - Faces quad (f v1 v2 v3 v4) → trianguladas em fan
  - Faces com normais/UVs (f v/vt/vn) → só usa o índice de vértice
  - Múltiplos objetos/grupos no mesmo OBJ
  - Normalização automática para caixa ±1 (activa por omissão)

Uso:
  python obj_to_3d.py <ficheiro.obj> [saida.3d] [--no-normalize]

  --no-normalize   Mantém as coordenadas originais do OBJ sem modificação.
                   Útil quando o modelo já está na escala correcta.
"""

import sys
import os


def parse_face_index(token: str) -> int:
    """Extrai o índice de vértice de um token 'v', 'v/vt' ou 'v/vt/vn'."""
    return int(token.split("/")[0])


def load_obj(path: str):
    """Lê um ficheiro OBJ e devolve lista de vértices e lista de faces (triângulos)."""
    vertices  = []  # list of (x, y, z)
    triangles = []  # list of (i0, i1, i2) — índices 0-based

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            token = parts[0].lower()

            if token == "v":
                x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                vertices.append((x, y, z))

            elif token == "f":
                # índices OBJ são 1-based; suporta negativo (relativo)
                n = len(vertices)
                indices = []
                for p in parts[1:]:
                    idx = parse_face_index(p)
                    idx = n + idx if idx < 0 else idx - 1  # → 0-based
                    indices.append(idx)

                # triangulação em fan: (0,1,2), (0,2,3), (0,3,4), ...
                for i in range(1, len(indices) - 1):
                    triangles.append((indices[0], indices[i], indices[i + 1]))

    return vertices, triangles


def normalize(vertices: list) -> list:
    """
    Centra o modelo na origem e escala-o para que caiba em ±1 em todos os eixos.
    Após esta operação, scale=1.0 no scatter corresponde a ~1 unidade de raio,
    tornando os parâmetros de escala intuitivos no contexto do sistema solar.
    """
    if not vertices:
        return vertices

    # 1. Centrar — subtrai o centróide
    cx = sum(v[0] for v in vertices) / len(vertices)
    cy = sum(v[1] for v in vertices) / len(vertices)
    cz = sum(v[2] for v in vertices) / len(vertices)
    centered = [(v[0] - cx, v[1] - cy, v[2] - cz) for v in vertices]

    # 2. Escalar — divide pelo maior extent absoluto
    max_extent = max(max(abs(v[0]), abs(v[1]), abs(v[2])) for v in centered)
    if max_extent == 0:
        return centered  # modelo degenerado, devolve centrado
    
    return [(v[0] / max_extent, v[1] / max_extent, v[2] / max_extent)
            for v in centered]


def write_3d(vertices: list, triangles: list, out_path: str):
    """Escreve o ficheiro .3d com vértices expandidos (sem índices)."""
    total_verts = len(triangles) * 3

    with open(out_path, "w") as f:
        f.write(f"{total_verts}\n")
        for i0, i1, i2 in triangles:
            for idx in (i0, i1, i2):
                x, y, z = vertices[idx]
                f.write(f"{x:.6f} {y:.6f} {z:.6f}\n")


def convert(obj_path: str, out_path: str = None, do_normalize: bool = True):
    if out_path is None:
        base = os.path.splitext(obj_path)[0]
        out_path = base + ".3d"

    print(f"A ler:    {obj_path}")
    verts, tris = load_obj(obj_path)
    print(f"  Vértices OBJ : {len(verts)}")
    print(f"  Triângulos   : {len(tris)}")
    print(f"  Vértices .3d : {len(tris) * 3}")

    if do_normalize:
        # Mostra o extent original para referência
        if verts:
            max_orig = max(max(abs(v[0]), abs(v[1]), abs(v[2])) for v in verts)
            print(f"  Extent original: ±{max_orig:.2f}  →  normalizado para ±1.0")
        verts = normalize(verts)
    else:
        print(f"  Normalização: desactivada (coordenadas originais mantidas)")

    write_3d(verts, tris, out_path)
    print(f"A escrever: {out_path}  ✓")


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flags = [a for a in sys.argv[1:] if a.startswith("--")]

    if len(args) < 1:
        print("Uso: python obj_to_3d.py <ficheiro.obj> [saida.3d] [--no-normalize]")
        sys.exit(1)

    obj_file     = args[0]
    out_file     = args[1] if len(args) >= 2 else None
    do_normalize = "--no-normalize" not in flags

    convert(obj_file, out_file, do_normalize)