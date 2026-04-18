#!/usr/bin/env python3
import math
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
XML_PATH = ROOT / "solar_system.xml"

SAMPLES = 120

# time -> eccentricity (approx. real Solar System values)
PLANET_ECCENTRICITY = {
    "8.8": 0.205,    # Mercury
    "22.6": 0.006,   # Venus
    "36.5": 0.016,   # Earth
    "68.6": 0.093,   # Mars
    "433": 0.048,    # Jupiter
    "1075": 0.054,   # Saturn
    "3066": 0.047,   # Uranus
    "6015": 0.008,   # Neptune
    "9056": 0.248,   # Pluto
}


def make_points(major_axis: float, eccentricity: float, samples: int) -> list[tuple[float, float, float]]:
    # Sun at one focus: x = a(cosE - e), z = b sinE
    minor_axis = major_axis * math.sqrt(max(0.0, 1.0 - eccentricity * eccentricity))
    pts = []
    for i in range(samples):
        ang = (2.0 * math.pi * i) / samples
        x = major_axis * (math.cos(ang) - eccentricity)
        z = minor_axis * math.sin(ang)
        pts.append((x, 0.0, z))

    # Catmull-Rom closure needs the first 3 repeated
    pts.extend(pts[:3])
    return pts


def fmt(v: float) -> str:
    s = f"{v:.3f}".rstrip("0").rstrip(".")
    return "0" if s == "-0" else s


def replace_translate_block(xml: str, orbit_time: str, eccentricity: float) -> tuple[str, bool]:
    pattern = re.compile(
        rf'(?P<indent>\s*)<translate\s+time="{re.escape(orbit_time)}"\s+align="false">\n(?P<body>.*?)\n(?P=indent)</translate>',
        re.DOTALL,
    )

    match = pattern.search(xml)
    if not match:
        return xml, False

    body = match.group("body")
    first_point = re.search(r'<point\s+x="([^"]+)"\s+y="([^"]+)"\s+z="([^"]+)"\s*/>', body)
    if not first_point:
        return xml, False

    try:
        major_axis = abs(float(first_point.group(1)))
    except ValueError:
        return xml, False

    points = make_points(major_axis, eccentricity, SAMPLES)
    point_indent = match.group("indent") + "    "
    new_lines = [
        f'{point_indent}<point x="{fmt(x)}" y="{fmt(y)}" z="{fmt(z)}" />'
        for x, y, z in points
    ]

    replacement = (
        f'{match.group("indent")}<translate time="{orbit_time}" align="false">\n'
        + "\n".join(new_lines)
        + f'\n{match.group("indent")}</translate>'
    )

    updated = xml[:match.start()] + replacement + xml[match.end():]
    return updated, True


def main() -> None:
    xml = XML_PATH.read_text(encoding="utf-8")
    updated = xml
    replaced = 0

    for orbit_time, ecc in PLANET_ECCENTRICITY.items():
        updated, changed = replace_translate_block(updated, orbit_time, ecc)
        if changed:
            replaced += 1

    XML_PATH.write_text(updated, encoding="utf-8")
    print(f"Updated {replaced} planetary orbit blocks in {XML_PATH}")


if __name__ == "__main__":
    main()
