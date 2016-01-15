DROP TABLE IF EXISTS `characters`;
DROP TABLE IF EXISTS `races`;
DROP TABLE IF EXISTS `locations`;
DROP TABLE IF EXISTS `players`;
DROP TABLE IF EXISTS `languages`;

CREATE TABLE `languages` (
  `id` smallint NOT NULL UNIQUE AUTO_INCREMENT,

  `languageCode` VARCHAR(4) NOT NULL, -- ISO 639-1 followed by capitalized ISO 3166-1 code for country
                                      -- e.g. enGB, enUS or deDE

  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

INSERT INTO `languages` VALUES(1, 'enGB');
INSERT INTO `languages` VALUES(2, 'deDE');

CREATE TABLE `players` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `email` varchar(255) NOT NULL UNIQUE,
  `password` CHAR(128) NOT NULL,
  `salt` CHAR(32) NOT NULL,

  `languageID` smallint NOT NULL DEFAULT 1,

  `locked` tinyint(1) NOT NULL DEFAULT 0, -- Indicates that a player has been banned/temporarily disabled

  `joinDate` DATETIME NOT NULL DEFAULT NOW(),
  `lastLogin` DATETIME NOT NULL DEFAULT NOW(),

  PRIMARY KEY (`id`),

  FOREIGN KEY (`languageID`)
    REFERENCES languages(`id`)
    ON UPDATE CASCADE
    ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

CREATE TABLE `locations` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `locationName` VARCHAR(50) NOT NULL,
  `knownFileName` VARCHAR(50) NOT NULL,
  `knownMD5Hash` VARCHAR(50) NOT NULL,

  `versionMajor` SMALLINT NOT NULL,
  `versionMinor` SMALLINT NOT NULL,
  `versionPatch` SMALLINT NOT NULL,

  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

CREATE TABLE `races` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `raceName` VARCHAR(25) NOT NULL,

  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

CREATE TABLE `characters` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `playerID` int(11) NOT NULL,

  `name` varchar(12) UNIQUE NOT NULL,
  `raceID` int(11) NOT NULL,

  `locationID` int(11) NOT NULL,

  `maxHP` int(11) NOT NULL,
  `strength` int(11) NOT NULL,
  `intelligence` int(11) NOT NULL,

  PRIMARY KEY (`id`),

  FOREIGN KEY (`playerID`)
    REFERENCES players(`id`)
    ON UPDATE CASCADE
    ON DELETE CASCADE,

  FOREIGN KEY(`locationID`)
    REFERENCES locations(`id`)
    ON UPDATE CASCADE
    ON DELETE RESTRICT,

  FOREIGN KEY(`raceID`)
    REFERENCES races(`id`)
    ON UPDATE CASCADE
    ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

-- INSERT INTO `players` VALUES (1, 'SgtCoDFish@example.com', 'testa');
-- INSERT INTO `players` VALUES (2, 'ad299@example.com', 'testa');

-- INSERT INTO `characters` VALUES (1, 1, 'Kurxka', 100, 5, 2);

INSERT INTO `races` VALUES (1, 'human');
INSERT INTO `races` VALUES (2, 'dog');

