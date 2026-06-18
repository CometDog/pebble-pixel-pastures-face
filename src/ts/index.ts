// TODO: I need to handle throwables more gracefully

const Clay = require("@rebble/clay");

const clayConfig = [
  {
    type: "heading",
    defaultValue: "Detail Settings",
  },
  {
    type: "section",
    items: [
      {
        type: "select",
        messageKey: "SETTINGS_DETAIL_TYPE",
        label: "Type of detail to display below time",
        defaultValue: "0",
        options: [
          { label: "Steps", value: "0" },
          { label: "Temperature", value: "1" },
        ],
      },
      {
        type: "select",
        messageKey: "SETTINGS_TEMPERATURE_UNIT",
        label: "Temperature unit",
        defaultValue: "0",
        options: [
          { label: "Celsius", value: "0" },
          { label: "Fahrenheit", value: "1" },
        ],
      },
      {
        type: "select",
        messageKey: "SETTINGS_PRIMARY_WEATHER_SOURCE",
        label: "Primary weather source",
        description: "OpenMeteo is the fallback in case of failures",
        options: [
          { label: "OpenMeteo", value: "0" },
          { label: "US National Weather Service", value: "1" },
        ],
      },
      {
        type: "input",
        messageKey: "SETTINGS_BIRTHDAY",
        label:
          "Your Birthday! Used to display a special season indicator image on your birthday! Year does not matter.",
        defaultValue: "",
        attributes: {
          type: "date",
        },
      },
    ],
  },
  {
    type: "submit",
    defaultValue: "Save Settings",
  },
];

const clay = new Clay(clayConfig);

const CUSTOM_SETTINGS_KEY = "pixel-pasture-settings";
interface CustomSettings {
  primary_weather_provider: WeatherProvider;
  fallback_ttl?: number;
}

enum WeatherProvider {
  OpenMeteo = 0,
  USNationalWeatherService = 1,
}

enum Season {
  Spring = 0,
  Summer = 1,
  Fall = 2,
  Winter = 3,
}

enum WeatherCondition {
  Rainy = 0,
  Sunny = 1,
  Snowy = 2,
  Windy = 3,
  Stormy = 4,
}

interface OpenMeteoWeatherData {
  current_weather: {
    weathercode: number;
    temperature: number;
  };
  daily: {
    sunrise: string[];
    sunset: string[];
  };
}

interface NWSPointResponse {
  properties: {
    forecastHourly: string;
    astronomicalData?: {
      sunrise: string;
      sunset: string;
    };
  };
}

interface NWSHourlyPeriod {
  startTime: string;
  endTime: string;
  temperature: number;
  temperatureUnit: string;
  shortForecast: string;
}

interface NWSHourlyResponse {
  properties: {
    periods: NWSHourlyPeriod[];
  };
}

interface WeatherSnapshot {
  weatherCondition: WeatherCondition;
  sunriseHour: number;
  sunsetHour: number;
  temperature: { c: number; f: number };
}

const sendAppMessage = (message: Record<string, any>) => {
  console.log("Sending message to watch: ", JSON.stringify(message));
  PebbleTS.sendAppMessage(message);
};

const getSeason = (latitiude: GeolocationCoordinates["latitude"]): Season => {
  const month = new Date().getMonth();
  let season: Season;
  if (month >= 2 && month <= 4) {
    season = Season.Spring;
  } else if (month >= 5 && month <= 7) {
    season = Season.Summer;
  } else if (month >= 8 && month <= 10) {
    season = Season.Fall;
  } else {
    season = Season.Winter;
  }

  if (latitiude < 0) {
    // Flip the season for southern hemisphere
    season = (season + 2) % 4;
  }

  return season;
};

// TODO: Consider rounding this with minutes for sunrise/sunset
const parseISODateHour = (isoString: string): number =>
  new Date(isoString).getHours();

const parseNWSCondition = (shortForecast: string): WeatherCondition => {
  const forecast = shortForecast.toLowerCase();
  if (forecast.includes("thunder") || forecast.includes("tornado"))
    return WeatherCondition.Stormy;
  if (
    forecast.includes("snow") ||
    forecast.includes("sleet") ||
    forecast.includes("ice") ||
    forecast.includes("freezing") ||
    forecast.includes("blizzard")
  )
    return WeatherCondition.Snowy;
  if (
    forecast.includes("rain") ||
    forecast.includes("drizzle") ||
    forecast.includes("shower")
  )
    return WeatherCondition.Rainy;
  if (forecast.includes("wind") || forecast.includes("blowing"))
    return WeatherCondition.Windy;
  return WeatherCondition.Sunny;
};

const parseOpenMeteoCondition = (wmoCode: number): WeatherCondition => {
  if (
    wmoCode <= 5 ||
    (wmoCode >= 10 && wmoCode <= 12) ||
    wmoCode === 28 ||
    (wmoCode >= 40 && wmoCode <= 49)
  )
    return WeatherCondition.Sunny;
  if (
    (wmoCode >= 6 && wmoCode <= 9) ||
    wmoCode === 18 ||
    wmoCode === 19 ||
    (wmoCode >= 30 && wmoCode <= 39)
  )
    return WeatherCondition.Windy;
  if (
    (wmoCode >= 22 && wmoCode <= 23) ||
    wmoCode === 26 ||
    (wmoCode >= 68 && wmoCode <= 79) ||
    (wmoCode >= 83 && wmoCode <= 86)
  )
    return WeatherCondition.Snowy;
  if (
    wmoCode === 13 ||
    wmoCode === 17 ||
    wmoCode === 27 ||
    wmoCode === 29 ||
    (wmoCode >= 87 && wmoCode <= 99)
  )
    return WeatherCondition.Stormy;
  if (
    (wmoCode >= 14 && wmoCode <= 16) ||
    (wmoCode >= 20 && wmoCode <= 21) ||
    (wmoCode >= 24 && wmoCode <= 25) ||
    (wmoCode >= 50 && wmoCode <= 67) ||
    (wmoCode >= 80 && wmoCode <= 82)
  )
    return WeatherCondition.Rainy;
  return WeatherCondition.Sunny;
};

const normalizeTemperature = ({
  celsius,
  fahrenheit,
}:
  | {
      celsius?: number;
      fahrenheit: number;
    }
  | {
      celsius: number;
      fahrenheit?: number;
    }): { c: number; f: number } => {
  if (celsius !== undefined) {
    const c = Math.round(celsius);
    return { c, f: Math.round((c * 9) / 5 + 32) };
  }
  if (fahrenheit !== undefined) {
    const f = Math.round(fahrenheit);
    return { c: Math.round(((f - 32) * 5) / 9), f };
  }
  throw new Error("Must provide either celsius or fahrenheit");
};

const getNWSWeatherSnapshot = async (
  coords: Pick<GeolocationCoordinates, "latitude" | "longitude">,
): Promise<WeatherSnapshot> => {
  console.debug("Making weather snapshot request to US NWS");
  const pointsUrl = `https://api.weather.gov/points/${coords.latitude.toFixed(
    4,
  )},${coords.longitude.toFixed(4)}`;

  const pointsRes = await fetch(pointsUrl, {
    headers: {
      Accept: "application/geo+json",
      "User-Agent": "(Pebble Pixel Pastures, scrums-malady6l@icloud.com)",
    },
  });
  if (!pointsRes.ok) {
    throw new Error(`NWS points request failed: ${pointsRes.statusText}`);
  }

  const pointsData: NWSPointResponse = await pointsRes.json();

  // TODO: I should handle the sunrise/sunset fallback more intelligently
  let sunriseHour = 6;
  let sunsetHour = 18;
  if (pointsData.properties.astronomicalData) {
    sunriseHour = parseISODateHour(
      pointsData.properties.astronomicalData.sunrise,
    );
    sunsetHour = parseISODateHour(
      pointsData.properties.astronomicalData.sunset,
    );
  }

  const hourlyRes = await fetch(pointsData.properties.forecastHourly, {
    headers: {
      Accept: "application/geo+json",
      "User-Agent": "(PebbleWatchface, contact@example.com)",
    },
  });
  if (!hourlyRes.ok) {
    throw new Error(`NWS hourly request failed: ${hourlyRes.statusText}`);
  }

  const hourlyData: NWSHourlyResponse = await hourlyRes.json();
  const now = new Date();
  const currentPeriod = hourlyData.properties.periods.find((period) => {
    const start = new Date(period.startTime);
    const end = new Date(period.endTime);
    return now >= start && now < end;
  });

  if (!currentPeriod) {
    throw new Error("No current hourly period found in NWS forecast");
  }

  const weatherCondition = parseNWSCondition(currentPeriod.shortForecast);

  const temperature =
    currentPeriod.temperatureUnit === "C"
      ? normalizeTemperature({ celsius: currentPeriod.temperature })
      : normalizeTemperature({ fahrenheit: currentPeriod.temperature });

  return { weatherCondition, sunriseHour, sunsetHour, temperature };
};

const getOpenMeteoWeatherSnapshot = async (
  coords: Pick<GeolocationCoordinates, "latitude" | "longitude">,
): Promise<WeatherSnapshot> => {
  console.debug("Making weather snapshot request to Open Meteo");
  const weatherResponse = await fetch(
    `https://api.open-meteo.com/v1/forecast?latitude=${coords.latitude}&longitude=${coords.longitude}&current_weather=true&daily=sunrise,sunset&timezone=auto`,
    {
      headers: {
        "User-Agent": "(Pebble Pixel Pastures, scrums-malady6l@icloud.com)",
      },
    },
  );
  if (!weatherResponse.ok) {
    throw new Error(
      `Error fetching weather data: ${weatherResponse.statusText}`,
    );
  }

  const weatherData: OpenMeteoWeatherData = await weatherResponse.json();

  const weatherCondition = parseOpenMeteoCondition(
    weatherData.current_weather.weathercode,
  );

  const sunriseHour = parseISODateHour(weatherData.daily.sunrise[0]);
  const sunsetHour = parseISODateHour(weatherData.daily.sunset[0]);

  const temperature = normalizeTemperature({
    celsius: weatherData.current_weather.temperature,
  });

  return { weatherCondition, sunriseHour, sunsetHour, temperature };
};

const getWeatherData = async (
  coords: Pick<GeolocationCoordinates, "latitude" | "longitude">,
): Promise<WeatherSnapshot> => {
  const settings = JSON.parse(
    localStorage.getItem(CUSTOM_SETTINGS_KEY) ?? "{}",
  );
  const weatherProvider = getWeatherProvider();
  if (weatherProvider === WeatherProvider.USNationalWeatherService) {
    try {
      return await getNWSWeatherSnapshot(coords);
    } catch (e) {
      console.error("NWS fetch failed, falling back to OpenMeteo:", e);
      localStorage.setItem(
        CUSTOM_SETTINGS_KEY,
        JSON.stringify({
          ...settings,
          fallback_ttl: Date.now() + 6.048e8 /* 1 week */,
        }),
      );
    }
  }

  // Open Meteo is always the fall back
  return getOpenMeteoWeatherSnapshot(coords);
};

const handlePositionSuccess = async (position: GeolocationPosition) => {
  const season = getSeason(position.coords.latitude);

  try {
    const weatherData = await getWeatherData(position.coords);
    sendAppMessage({
      type: "weatherUpdate",
      season,
      weatherCondition: weatherData.weatherCondition,
      sunriseHour: weatherData.sunriseHour,
      sunsetHour: weatherData.sunsetHour,
      temperatureC: weatherData.temperature.c,
      temperatureF: weatherData.temperature.f,
    });
  } catch (e) {
    console.error("Failed to get weather data, skipping watch update:", e);
  }
};

const handlePositionError = (error: GeolocationPositionError) => {
  console.error("Error getting geolocation: ", error);
};

const weatherRefresh = () => {
  if (navigator.geolocation) {
    window.navigator.geolocation.getCurrentPosition(
      handlePositionSuccess,
      handlePositionError,
    );
  }
};

const getWeatherProvider = (): WeatherProvider => {
  const settings: CustomSettings = JSON.parse(
    localStorage.getItem(CUSTOM_SETTINGS_KEY) ?? "{}",
  );
  if (settings.primary_weather_provider) {
    if (
      settings.fallback_ttl === undefined ||
      settings.fallback_ttl < Date.now()
    ) {
      localStorage.setItem(
        CUSTOM_SETTINGS_KEY,
        JSON.stringify({ ...settings, fallback_ttl: 0 }),
      );
      return settings.primary_weather_provider;
    } else {
      return WeatherProvider.OpenMeteo;
    }
  } else {
    return WeatherProvider.OpenMeteo;
  }
};

const setWeatherProvider = (newWeatherProvider: WeatherProvider) => {
  const localSettings: CustomSettings = JSON.parse(
    localStorage.getItem(CUSTOM_SETTINGS_KEY) ?? "{}",
  );
  const currentWeatherProvider =
    localSettings?.primary_weather_provider ?? null;
  if (
    currentWeatherProvider !== null &&
    newWeatherProvider === currentWeatherProvider
  ) {
    return;
  }

  localStorage.setItem(
    CUSTOM_SETTINGS_KEY,
    JSON.stringify({
      ...localSettings,
      primary_weather_provider: newWeatherProvider,
    } as CustomSettings),
  );

  weatherRefresh();
};

Pebble.addEventListener("ready", async (_) => {
  console.log("PebbleKit JS ready!");
  const settings = JSON.parse(localStorage.getItem("clay-settings") ?? "{}");
  const weatherProvider =
    settings?.["SETTINGS_PRIMARY_WEATHER_SOURCE"] === "1"
      ? WeatherProvider.USNationalWeatherService
      : WeatherProvider.OpenMeteo;
  setWeatherProvider(weatherProvider);
  sendAppMessage({ type: "ready" });
});

// @ts-ignore
Pebble.addEventListener("webviewclosed", async (e: any) => {
  const settings = clay.getSettings(e.response);
  const weatherProvider =
    settings?.["10010"] === "1"
      ? WeatherProvider.USNationalWeatherService
      : WeatherProvider.OpenMeteo;
  setWeatherProvider(weatherProvider);
});

Pebble.addEventListener("appmessage", async (e) => {
  console.log("Received message from watch: ", JSON.stringify(e.payload));
  if (e.payload.type === "requestWeatherUpdate") {
    weatherRefresh();
  }
});
