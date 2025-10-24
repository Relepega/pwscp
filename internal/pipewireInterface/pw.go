package pipewireInterface

// This works as of pipewire --version:
// Compiled with libpipewire 1.4.8
// Linked with libpipewire 1.4.8

import (
	"fmt"
	"math"
	"os/exec"
	"slices"
	"strconv"
	"strings"
)

const (
	AutoValue   = "(Auto)"
	NilValue    = ""
	singleQuote = '\''
)

type PipewireOption struct {
	// update: id:0 key:'log.level' value:'2' type:''
	Id    int
	Key   string
	value string
	Type  string
}

func pwOptTokenParser(opt *PipewireOption, key string) error {
	// $ pw-metadata -n settings 0 clock.force-quantum
	// Found "settings" metadata 31
	// update: id:0 key:'clock.force-quantum' value:'' type:''

	out, err := exec.Command("pw-metadata", "-n", "settings", "0", key).CombinedOutput()
	if err != nil {
		return fmt.Errorf("NewPipewireOption(key %v): %v", key, err)
	}

	str := strings.Split(string(out), "\n")[1]
	str = strings.TrimPrefix(str, "update: ")

	strlen := len(str)
	l := 0
	r := 0

	k := ""
	v := ""

	for r < strlen {
		if str[r] == ':' {
			k = strings.TrimSpace(str[l:r])

			r++ // skips ':'

			// skips opening singlequote
			if str[r] == singleQuote {
				r++
			}

			l = r

			if k == "id" {
				for {
					if str[r] == ' ' {
						v = str[l:r]

						r = r + 1 // skips ' '
						l = r

						k = ""
						v = ""

						break
					}

					r++
				}

				continue
			}

			continue
		}

		if str[r] == singleQuote {
			v = NilValue

			v = str[l:r]

			if v == "" {
				v = NilValue
			}

			// skips outer singlequote if not out of bound
			if r+1 < strlen && str[r+1] == singleQuote {
				r++
			}

			// skips ' ' if not out of bound
			if r+1 < strlen && str[r+1] == ' ' {
				r++
			}

			l = r
		}

		// assign to struct
		if k != "" && v != "" {
			switch k {
			case "id":
				id, _ := strconv.Atoi(v)
				opt.Id = id

			case "key":
				opt.Key = v

			case "value":
				if key == "clock.allowed-rates" {
					opt.value = v[2 : len(v)-2]
				} else {
					opt.value = v
				}

			case "type":
				opt.Type = v
			}

			k = ""
			v = ""
		}

		r++
	}

	return nil
}

func NewPipewireOption(key string) (*PipewireOption, error) {
	opt := &PipewireOption{}

	err := pwOptTokenParser(opt, key)
	if err != nil {
		return nil, err
	}

	return opt, err
}

func (po *PipewireOption) Value() string {
	if po.value == NilValue {
		return AutoValue
	}

	return po.value
}

func (po *PipewireOption) Set(value string) error {
	valueArg := value

	if value == AutoValue {
		valueArg = NilValue
	}

	err := exec.Command("pw-metadata", "-n", "settings", "0", po.Key, valueArg).Run()
	if err != nil {
		return err
	}

	po.value = valueArg

	return nil
}

func (po *PipewireOption) Update() error {
	return pwOptTokenParser(po, po.Key)
}

type PipewireOptions struct {
	Log struct {
		// Controls Pipewire's logging Level
		Level *PipewireOption
	}

	Clock struct {
		// Controls the Sampling Rate
		Rate *PipewireOption
		// Allowed sample rates at user level
		AllowedRates *PipewireOption
		// Controls the Buffer Size
		Quantum *PipewireOption
		// Minimum allowed buffer size
		MinQuantum *PipewireOption
		// Maximum allowed buffer size
		MaxQuantum *PipewireOption
		// Forces all applications to use the specified buffer size
		ForceQuantum *PipewireOption
		// Forces the aggregated pipewire output to use the specified sample rate
		//
		// NOTE: All the apps will use this Sample rate if they are allowed to, else they will fallback to use their preferred one.
		ForceRate *PipewireOption

		collection []*PipewireOption
	}

	collection []*PipewireOption
}

func NewPipewireOptions() (*PipewireOptions, error) {
	opts := &PipewireOptions{}

	keys := []string{
		"log.level",
		"clock.rate",
		"clock.allowed-rates",
		"clock.quantum",
		"clock.min-quantum",
		"clock.max-quantum",
		"clock.force-quantum",
		"clock.force-rate",
	}

	for _, k := range keys {
		opt, err := NewPipewireOption(k)
		if err != nil {
			return nil, err
		}

		switch k {
		case "log.level":
			opts.Log.Level = opt

		case "clock.rate":
			opts.Clock.Rate = opt

		case "clock.allowed-rates":
			opts.Clock.AllowedRates = opt

		case "clock.quantum":
			opts.Clock.Quantum = opt

		case "clock.min-quantum":
			opts.Clock.MinQuantum = opt

		case "clock.max-quantum":
			opts.Clock.MaxQuantum = opt

		case "clock.force-quantum":
			opts.Clock.ForceQuantum = opt

		case "clock.force-rate":
			opts.Clock.ForceRate = opt
		}

		opts.collection = append(opts.collection, opt)

	}

	return opts, nil
}

func (pwopts *PipewireOptions) CurrentSampleRate() string {
	if pwopts.Clock.ForceRate.value == NilValue {
		return AutoValue
	}

	return pwopts.Clock.ForceRate.value
}

func (pwopts *PipewireOptions) CurrentBufferSize() string {
	if pwopts.Clock.ForceQuantum.value == NilValue {
		return AutoValue
	}

	return pwopts.Clock.ForceQuantum.value
}

func (pwopts *PipewireOptions) AvailableSampleRates() []string {
	sr := []string{AutoValue}

	sr = append(sr, strings.Split(pwopts.Clock.AllowedRates.Value(), ", ")...)

	return sr
}

func (pwopts *PipewireOptions) AvailableBufferSizes() []string {
	bs := make([]string, 1)

	bs[0] = AutoValue

	i := 1
	lastPower := 0
	maxPower, err := strconv.Atoi(pwopts.Clock.MaxQuantum.Value())
	if err != nil {
		maxPower = 8192
	}

	minPower, err := strconv.Atoi(pwopts.Clock.MinQuantum.Value())
	if err != nil {
		minPower = 8
	}

	for lastPower < maxPower {
		p := int(math.Pow(2, float64(i)))

		if p >= minPower {
			bs = append(bs, strconv.Itoa(p))
		}

		lastPower = p

		i++
	}

	return bs
}

func (pwopts *PipewireOptions) Update(key string) error {
	for i := range len(pwopts.collection) {
		if pwopts.collection[i].Key == key {
			return pwopts.collection[i].Update()
		}
	}

	return fmt.Errorf("PipewireOptions: No option with key '%s' has been found", key)
}

func (pwopts *PipewireOptions) UpdateAll() error {
	for i := range len(pwopts.collection) {
		err := pwopts.collection[i].Update()
		if err != nil {
			return err
		}
	}

	return nil
}

func (pwopts *PipewireOptions) IsSampleRateAvailable(sr string) bool {
	return strings.Contains(pwopts.Clock.AllowedRates.value, sr)
}

func (pwopts *PipewireOptions) IsBufferSizeAvailable(bs string) bool {
	availableBS := pwopts.AvailableBufferSizes()

	return slices.Contains(availableBS, bs)
}
