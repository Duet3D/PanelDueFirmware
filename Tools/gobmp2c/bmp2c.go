package main

import (
	"bufio"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"

	"golang.org/x/image/bmp"
)

func main() {
	var binaryOutput bool
	flag.BoolVar(&binaryOutput, "binary", false, "Binary output")
	flag.Parse()

	files := flag.Args()

	for _, file := range files {
		f, err := os.Open(file)
		if err != nil {
			panic(err)
		}
		defer f.Close()

		b, err := bmp.Decode(f)
		if err != nil {
			panic(err)
		}

		if binaryOutput {
			buf := bufio.NewWriter(os.Stdout)
			defer buf.Flush()
			writeBinary(buf, uint16(b.Bounds().Max.X))
			writeBinary(buf, uint16(b.Bounds().Max.Y))

			repeatCount := uint32(0)
			lastData := uint16(0)
			for y := b.Bounds().Max.Y; y > b.Bounds().Min.Y; {
				y--
				for x := b.Bounds().Min.X; x < b.Bounds().Max.X; x++ {
					c := b.At(x, y)
					oc := convertTo16BitColor(c.RGBA())
					if repeatCount > 0 && (lastData != oc || repeatCount == 1<<16) {
						writeBinary(buf, uint16(repeatCount-1))
						writeBinary(buf, lastData)
						repeatCount = 0
					}
					lastData = oc
					repeatCount++
					// Last pixel
					if x+1 == b.Bounds().Max.X && y == 0 {
						writeBinary(buf, uint16(repeatCount-1))
						writeBinary(buf, lastData)
					}
				}
			}
		} else {
			pixelCount := 0
			fmt.Printf("extern const uint8_t %s[] =\n", strings.TrimSuffix(strings.TrimSuffix(file, "_21h.bmp"), "_30h.bmp"))
			fmt.Printf("{\t%d, %d,\t// width, height\n\t", b.Bounds().Dx(), b.Bounds().Dy())
			for x := b.Bounds().Min.X; x < b.Bounds().Max.X; x++ {
				for y := b.Bounds().Min.Y; y < b.Bounds().Max.Y; y++ {
					c := b.At(x, y)
					if pixelCount%2 == 0 {
						fmt.Print("0x")
					}
					fmt.Print(getPaletteIndex(c.RGBA()))
					pixelCount++
					if pixelCount%2 == 0 {
						fmt.Print(", ")
						// wrap every 24 pixels
						if pixelCount%24 == 0 {
							fmt.Print("\n\t")
						}
					}
				}
			}
			// Pad with 0 for uneven pixel count
			if pixelCount%2 == 1 {
				fmt.Print("0")
			}
			fmt.Println("};")
		}
	}
}

func getPaletteIndex(r, g, b, a uint32) int {

	// 0x0000, 0xffff, 0x20e4, 0xffdf, 0x18e3, 0xf79e, 0xc986, 0xd30c,
	// 0xc103, 0xff52, 0xfffb, 0x4569, 0x9492, 0x0000, 0x0000, 0x0000

	switch convertTo16BitColor(r, g, b, a) {
	case 0xffff: // e.g. 0xffffff
		return 1
	case 0x20e4: // e.g. 0x201c20
		return 2
	case 0xffdf: // e.g. 0xf8b8f8
		return 3
	case 0x18e3: // e.g. 0x181c18
		return 4
	case 0xf79e: // e.g. 0xf0b0f0
		return 5
	case 0xc986: // e.g. 0xc83030
		return 6
	case 0xd30c: // e.g. 0xd02060
		return 7
	case 0xc103: // e.g. 0xc02018
		return 8
	case 0xff52: // e.g. 0xf8a890
		return 9
	case 0xfffb: // e.g. 0xf8bcd8
		return 10
	case 0x4569: // e.g. 0x40ac48
		return 11
	case 0x9492: // e.g. 0x909090
		return 12
	default:
		return 0
	}
}

func convertTo16BitColor(r, g, b, a uint32) uint16 {
	return uint16((r&0xF8)<<8) | uint16((g&0xFC)<<3) | uint16((b&0xF8)>>3)
}

func writeBinary(w io.Writer, data interface{}) error {
	return binary.Write(w, binary.LittleEndian, data)
}
