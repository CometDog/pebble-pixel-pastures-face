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

new Clay(clayConfig);

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

const getWeatherData = async (
  coords: Pick<GeolocationCoordinates, "latitude" | "longitude">,
): Promise<{
  weatherCondition: WeatherCondition;
  sunriseHour: number;
  sunsetHour: number;
  temperature: {
    c: number;
    f: number;
  };
}> => {
  // Open Meteo API Call
  const rawWeatherConditions = await fetch(
    `https://api.open-meteo.com/v1/forecast?latitude=${coords.latitude}&longitude=${coords.longitude}&current_weather=true&daily=sunrise,sunset&timezone=auto`,
  );
  if (!rawWeatherConditions.ok) {
    console.error(
      "Error fetching weather data: ",
      rawWeatherConditions.statusText,
    );
    return {
      weatherCondition: WeatherCondition.Sunny,
      sunriseHour: 0,
      sunsetHour: 0,
      temperature: { c: 0, f: 32 },
    };
  }

  const weatherData: OpenMeteoWeatherData = await rawWeatherConditions.json();

  // Map WMO 4677 weather codes to our WeatherCondition enum
  const wmoCode = weatherData.current_weather.weathercode;
  let weatherCondition: WeatherCondition;
  if (
    wmoCode <= 5 ||
    (wmoCode >= 10 && wmoCode <= 12) ||
    wmoCode === 28 ||
    (wmoCode >= 40 && wmoCode <= 49)
  ) {
    // Clear, clouds, haze, mist, shallow fog, fog
    weatherCondition = WeatherCondition.Sunny;
  } else if (
    (wmoCode >= 6 && wmoCode <= 9) ||
    wmoCode === 18 ||
    wmoCode === 19 ||
    (wmoCode >= 30 && wmoCode <= 39)
  ) {
    // Dust/sand raised by wind, squalls, funnel clouds, dust/sandstorms, blowing snow
    weatherCondition = WeatherCondition.Windy;
  } else if (
    (wmoCode >= 22 && wmoCode <= 23) ||
    wmoCode === 26 ||
    (wmoCode >= 68 && wmoCode <= 79) ||
    (wmoCode >= 83 && wmoCode <= 86)
  ) {
    // Snow, rain+snow mix, ice pellets, snow showers
    weatherCondition = WeatherCondition.Snowy;
  } else if (
    wmoCode === 13 ||
    wmoCode === 17 ||
    wmoCode === 27 ||
    wmoCode === 29 ||
    (wmoCode >= 87 && wmoCode <= 99)
  ) {
    // Lightning, thunderstorms, hail, snow pellets/small hail
    weatherCondition = WeatherCondition.Stormy;
  } else if (
    (wmoCode >= 14 && wmoCode <= 16) ||
    (wmoCode >= 20 && wmoCode <= 21) ||
    (wmoCode >= 24 && wmoCode <= 25) ||
    (wmoCode >= 50 && wmoCode <= 67) ||
    (wmoCode >= 80 && wmoCode <= 82)
  ) {
    // 14-16 (precip in sight), 20-21/24-25 (drizzle/rain recent), 50-67 (drizzle/rain), 80-82 (rain showers)
    weatherCondition = WeatherCondition.Rainy;
  } else {
    // Default to sunny for unrecognized codes
    weatherCondition = WeatherCondition.Sunny;
  }
  const sunriseHour = new Date(weatherData.daily.sunrise[0]).getHours();
  const sunsetHour = new Date(weatherData.daily.sunset[0]).getHours();

  const temperatureC = weatherData.current_weather.temperature;
  const temperatureF = Math.round((temperatureC * 9) / 5 + 32);

  return {
    weatherCondition,
    sunriseHour,
    sunsetHour,
    temperature: { c: temperatureC, f: temperatureF },
  };
};

const handlePositionSuccess = async (position: GeolocationPosition) => {
  const season = getSeason(position.coords.latitude);

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
};

const handlePositionError = (error: GeolocationPositionError) => {
  console.error("Error getting geolocation: ", error);
};

Pebble.addEventListener("ready", async (_) => {
  console.log("PebbleKit JS ready!");
  sendAppMessage({ type: "ready" });
});

Pebble.addEventListener("appmessage", async (e) => {
  console.log("Received message from watch: ", JSON.stringify(e.payload));
  if (e.payload.type === "requestWeatherUpdate") {
    if (navigator.geolocation) {
      // Start a loop to update weather every 30 minutes
      (function () {
        window.navigator.geolocation.getCurrentPosition(
          handlePositionSuccess,
          handlePositionError,
        );
        setTimeout(arguments.callee, 1800000); // 30 minutes
      })();
    }
  }
});
